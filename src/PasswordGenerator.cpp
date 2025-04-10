#include "./include/PasswordGenerator.h"
#include "./include/generator.h"

#include <QClipboard>
#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDate>
#include <QLabel>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <set>

const int MAX_SAFE_LENGTH = 1024;

PasswordGenerator::PasswordGenerator(QWidget* parent)
    : QMainWindow(parent),
    m_updatePending(false),
    m_debounceTimer(new QTimer(this)),
    m_passwordGeneration(false)
{
    ui.setupUi(this);
    setWindowTitle("Password Generator");
    setWindowIcon(QIcon(":/resources/icons/app-icon.png"));

    ui.lenght->setValue(8);
    ui.lenght_slider->setValue(8);
    ui.all->setChecked(true);
    ui.numbers->setChecked(true);
    ui.lowercase->setChecked(true);

    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);
    connect(m_debounceTimer, &QTimer::timeout, this, &PasswordGenerator::generatePasswordSafely);

    connect(ui.lenght_slider, &QSlider::valueChanged, ui.lenght, &QSpinBox::setValue);
    connect(ui.lenght, QOverload<int>::of(&QSpinBox::valueChanged), ui.lenght_slider, &QSlider::setValue);

    connect(ui.lenght, QOverload<int>::of(&QSpinBox::valueChanged), this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.lenght_slider, &QSlider::valueChanged, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.all, &QRadioButton::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.easy_read, &QRadioButton::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.easy_say, &QRadioButton::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.numbers, &QCheckBox::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.lowercase, &QCheckBox::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.uppercase, &QCheckBox::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.special_chars, &QCheckBox::toggled, this, &PasswordGenerator::schedulePasswordUpdate);
    connect(ui.excluded_chars, &QLineEdit::textChanged, this, &PasswordGenerator::schedulePasswordUpdate);

    connect(ui.reset_button, &QPushButton::clicked, this, &PasswordGenerator::resetSettings);
    connect(ui.copy_button, &QPushButton::clicked, this, &PasswordGenerator::copyPassword);

    ui.lenght->setMaximum(MAX_SAFE_LENGTH);
    ui.lenght_slider->setMaximum(100);

    schedulePasswordUpdate();
}

PasswordGenerator::~PasswordGenerator()
{
    m_passwordGeneration = false;
    if (m_updateFuture.isRunning()) {
        m_updateFuture.cancel();
        m_updateFuture.waitForFinished();
    }
}

void PasswordGenerator::schedulePasswordUpdate()
{
    m_updatePending = true;
    m_debounceTimer->start();
}

void PasswordGenerator::generatePasswordSafely()
{
    if (!m_updatePending || m_passwordGeneration) {
        return;
    }

    m_updatePending = false;
    m_passwordGeneration = true;

    setUIEnabled(false);

    ui.result->setText("Generating password...");

    PasswordParams params;
    params.length = ui.lenght->value();
    params.easyToRead = ui.easy_read->isChecked();
    params.easyToSay = ui.easy_say->isChecked();
    params.includeNumbers = ui.numbers->isChecked();
    params.includeUpper = ui.uppercase->isChecked();
    params.includeLower = ui.lowercase->isChecked();
    params.includeSymbols = ui.special_chars->isChecked();
    params.excludedChars = ui.excluded_chars->text().toStdString();

    if (ui.all->isChecked()) {
        params.easyToRead = false;
        params.easyToSay = false;
    }

    if (!params.includeNumbers && !params.includeUpper && !params.includeLower && !params.includeSymbols) {
        params.includeLower = true;
        ui.lowercase->setChecked(true);
    }

    if ((params.easyToRead || params.easyToSay) && params.length > 100) {
        params.length = 100;
        QMetaObject::invokeMethod(this, "updateLengthUI", Qt::QueuedConnection, Q_ARG(int, 100));
    }

    if (m_updateFuture.isRunning()) {
        m_updateFuture.cancel();
        m_updateFuture.waitForFinished();
    }

    m_updateFuture = QtConcurrent::run([this, params]() {
        QElapsedTimer timer;
        timer.start();
        const qint64 timeout = 5000;

        try {
            std::string password;

            if ((params.easyToRead || params.easyToSay) && params.length > 50) {
                int remainingLength = params.length;
                while (remainingLength > 0 && m_passwordGeneration) {
                    int chunkSize = std::min(remainingLength, 50);
                    std::string chunk = generatePassword(
                        chunkSize,
                        params.easyToRead,
                        params.easyToSay,
                        params.includeNumbers,
                        params.includeUpper,
                        params.includeLower,
                        params.includeSymbols,
                        params.excludedChars
                    );

                    password += chunk;
                    remainingLength -= chunkSize;

                    if (timer.elapsed() > timeout) {
                        throw std::runtime_error("Password generation timed out. Try with fewer constraints or a shorter length.");
                    }
                }
            }
            else {
                password = generatePassword(
                    params.length,
                    params.easyToRead,
                    params.easyToSay,
                    params.includeNumbers,
                    params.includeUpper,
                    params.includeLower,
                    params.includeSymbols,
                    params.excludedChars
                );

                if (timer.elapsed() > timeout) {
                    throw std::runtime_error("Password generation timed out. Try with fewer constraints or a shorter length.");
                }
            }

            if (!m_passwordGeneration) {
                return;
            }

            QMetaObject::invokeMethod(this, "applyGeneratedPassword",
                Qt::QueuedConnection,
                Q_ARG(QString, QString::fromStdString(password)));

        }
        catch (const std::exception& e) {
            if (m_passwordGeneration) {
                QMetaObject::invokeMethod(this, "handlePasswordError",
                    Qt::QueuedConnection,
                    Q_ARG(QString, QString(e.what())));
            }
        }

        QMetaObject::invokeMethod(this, "finishGeneration", Qt::QueuedConnection);
        });
}

void PasswordGenerator::finishGeneration()
{
    m_passwordGeneration = false;
    setUIEnabled(true);
}

void PasswordGenerator::updateLengthUI(int value)
{
    ui.lenght->setValue(value);
    ui.lenght_slider->setValue(std::min(value, ui.lenght_slider->maximum()));
}

void PasswordGenerator::applyGeneratedPassword(const QString& password)
{
    ui.result->setText(password);

    int strength = evaluatePasswordStrength(password.toStdString());
    updatePasswordStrengthBar(strength);
}

void PasswordGenerator::handlePasswordError(const QString& errorMsg)
{
    ui.result->setText(tr("Error: %1").arg(errorMsg));

    QMessageBox::warning(this, tr("Password Generation Error"),
        tr("Could not generate password with current settings.\n\n%1").arg(errorMsg));

    updatePasswordStrengthBar(0);
}

void PasswordGenerator::setUIEnabled(bool enabled)
{
    ui.lenght->setEnabled(enabled);
    ui.lenght_slider->setEnabled(enabled);
    ui.all->setEnabled(enabled);
    ui.easy_read->setEnabled(enabled);
    ui.easy_say->setEnabled(enabled);
    ui.numbers->setEnabled(enabled);
    ui.uppercase->setEnabled(enabled);
    ui.lowercase->setEnabled(enabled);
    ui.special_chars->setEnabled(enabled);
    ui.excluded_chars->setEnabled(enabled);
    ui.reset_button->setEnabled(enabled);
}

void PasswordGenerator::resetSettings()
{
    m_passwordGeneration = false;
    if (m_updateFuture.isRunning()) {
        m_updateFuture.cancel();
        m_updateFuture.waitForFinished();
    }

    ui.lenght->setValue(8);
    ui.lenght_slider->setValue(8);
    ui.all->setChecked(true);
    ui.easy_read->setChecked(false);
    ui.easy_say->setChecked(false);
    ui.numbers->setChecked(true);
    ui.uppercase->setChecked(false);
    ui.lowercase->setChecked(true);
    ui.special_chars->setChecked(false);
    ui.excluded_chars->setText("");

    schedulePasswordUpdate();
}

void PasswordGenerator::updatePasswordStrengthBar(int strengthLevel) {
    QStringList colors = {
        "#ff0000", 
        "#ff4500", 
        "#ff8c00", 
        "#ffd700", 
        "#adff2f",
        "#32cd32", 
        "#008000", 
        "#006400", 
        "#004d00"
    };

    QStringList levelNames = {
        "Might as well use 'password'",
        "My grandma could crack this",
        "Meh, better than nothing",
        "Getting there...",
        "Decent enough",
        "Now we're talking!",
        "Impressive security",
        "Fort Knox approves",
        "Even the NSA is sweating"
    };

    int expandedLevel = strengthLevel * 2;
    if (strengthLevel > 0 && expandedLevel < 8) {
        if (QRandomGenerator::global()->bounded(2) == 1) {
            expandedLevel += 1;
        }
    }
    expandedLevel = std::max(0, std::min(expandedLevel, 8));

    int value = (expandedLevel + 1) * 100 / 9;
    ui.robustness->setValue(value);

    QString color = colors[expandedLevel];
    QString style = QString(R"(
        QProgressBar {
            border: none;
            border-radius: 8px;
            background-color: #ddd;
            height: 16px;
            color: transparent;
            text-align: center;
        }
        
        QProgressBar::chunk {
            border-radius: 8px;
            background-color: %1;
            /* Add gradient effect */
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2);
        }
    )").arg(color).arg(expandedLevel < 8 ? colors[expandedLevel + 1] : color);

    ui.robustness->setStyleSheet(style);
    ui.robustness->setFormat("");
    ui.robustness->setTextVisible(false);

    QString labelText = levelNames[expandedLevel];

    ui.robustness_label->setText(labelText);
    ui.robustness_label->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));

    QString tooltip = getPasswordImprovementTips(evaluatePasswordDetails());
    ui.robustness->setToolTip(tooltip);
    ui.robustness_label->setToolTip(tooltip);
}

int PasswordGenerator::evaluatePasswordStrength(const std::string& password) {
    if (password.empty() || password.substr(0, 5) == "Error") {
        return 0;
    }

    int score = 0;

    if (password.length() >= 8) score += 2;
    if (password.length() >= 10) score += 1;
    if (password.length() >= 12) score += 2;
    if (password.length() >= 16) score += 2;
    if (password.length() >= 20) score += 1;

    bool hasDigit = false;
    bool hasLower = false;
    bool hasUpper = false;
    bool hasSpecial = false;
    int uniqueChars = 0;
    std::set<char> charSet;

    for (char c : password) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isdigit(uc)) hasDigit = true;
        if (std::islower(uc)) hasLower = true;
        if (std::isupper(uc)) hasUpper = true;
        if (std::ispunct(uc)) hasSpecial = true;
        charSet.insert(c);
    }

    uniqueChars = charSet.size();

    if (hasDigit) score += 1;
    if (hasLower) score += 1;
    if (hasUpper) score += 1;
    if (hasSpecial) score += 2;
    if (uniqueChars >= 8) score += 1;
    if (uniqueChars >= 12) score += 1;
    if (uniqueChars >= 16) score += 1;

    int penalties = 0;

    bool hasSequential = false;
    for (size_t i = 0; i < password.length() - 2; i++) {
        if ((password[i + 1] == password[i] + 1) && (password[i + 2] == password[i] + 2)) {
            hasSequential = true;
            break;
        }
    }
    if (hasSequential) penalties += 1;

    bool hasRepeated = false;
    for (size_t i = 0; i < password.length() - 2; i++) {
        if (password[i] == password[i + 1] && password[i] == password[i + 2]) {
            hasRepeated = true;
            break;
        }
    }
    if (hasRepeated) penalties += 1;

    score = std::max(0, score - penalties);

    return std::min(static_cast<int>(score / 5), 4);
}

PasswordDetails PasswordGenerator::evaluatePasswordDetails() {
    PasswordDetails details;
    QString password = ui.result->text();

    if (password.isEmpty() || password.startsWith("Error:") ||
        password == "Generating password...") {
        return details;
    }

    std::string stdPassword = password.toStdString();
    details.length = static_cast<int>(stdPassword.length());

    std::set<char> charSet;
    for (char c : stdPassword) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isdigit(uc)) details.hasDigits = true;
        if (std::islower(uc)) details.hasLowercase = true;
        if (std::isupper(uc)) details.hasUppercase = true;
        if (std::ispunct(uc)) details.hasSpecialChars = true;
        charSet.insert(c);
    }

    details.uniqueChars = static_cast<int>(charSet.size());

    for (size_t i = 0; i < stdPassword.length() - 2; i++) {
        if ((stdPassword[i + 1] == stdPassword[i] + 1) && (stdPassword[i + 2] == stdPassword[i] + 2)) {
            details.hasSequentialChars = true;
            break;
        }
    }

    for (size_t i = 0; i < stdPassword.length() - 2; i++) {
        if (stdPassword[i] == stdPassword[i + 1] && stdPassword[i] == stdPassword[i + 2]) {
            details.hasRepeatedChars = true;
            break;
        }
    }

    return details;
}

QString PasswordGenerator::getPasswordImprovementTips(const PasswordDetails& details) {
    QString tips;

    if (!details.hasDigits) {
        tips += "- Add some numbers\n";
    }
    if (!details.hasUppercase) {
        tips += "- Add uppercase letters\n";
    }
    if (!details.hasSpecialChars) {
        tips += "- Add special characters (!@#$%^&*)\n";
    }
    if (details.length < 12) {
        tips += "- Make it longer (aim for 12+ characters)\n";
    }
    if (details.hasSequentialChars) {
        tips += "- Avoid sequential characters (abc, 123)\n";
    }
    if (details.hasRepeatedChars) {
        tips += "- Avoid repeated characters (aaa, 111)\n";
    }

    if (tips.isEmpty()) {
        return "Great password! No improvements needed.";
    }
    else {
        return "Tips to improve your password:\n" + tips;
    }
}

void PasswordGenerator::copyPassword()
{
    QString password = ui.result->text();

    if (password.isEmpty() || password.startsWith("Error:") ||
        password == "Generating password...") {
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(password);
}