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

PasswordGenerator::PasswordGenerator(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle("Password Generator");
    setWindowIcon(QIcon(":/resources/icons/app-icon.png"));

    ui.lenght->setValue(8);
    ui.lenght_slider->setValue(8);
    ui.all->setChecked(true);
    ui.numbers->setChecked(true);
    ui.lowercase->setChecked(true);

    connect(ui.lenght_slider, &QSlider::valueChanged, ui.lenght, &QSpinBox::setValue);
    connect(ui.lenght, QOverload<int>::of(&QSpinBox::valueChanged), ui.lenght_slider, &QSlider::setValue);

    connect(ui.lenght, QOverload<int>::of(&QSpinBox::valueChanged), this, &PasswordGenerator::updatePassword);
    connect(ui.lenght_slider, &QSlider::valueChanged, this, &PasswordGenerator::updatePassword);
    connect(ui.all, &QRadioButton::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.easy_read, &QRadioButton::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.easy_say, &QRadioButton::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.numbers, &QCheckBox::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.lowercase, &QCheckBox::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.uppercase, &QCheckBox::toggled, this, &PasswordGenerator::updatePassword);
    connect(ui.special_chars, &QCheckBox::toggled, this, &PasswordGenerator::updatePassword);

    connect(ui.reset_button, &QPushButton::clicked, this, &PasswordGenerator::resetSettings);
    connect(ui.copy_button, &QPushButton::clicked, this, &PasswordGenerator::copyPassword);

    updatePassword();
}

PasswordGenerator::~PasswordGenerator()
{
}

void PasswordGenerator::updatePassword()
{
    int length = ui.lenght->value();
    bool easyToRead = ui.easy_read->isChecked();
    bool easyToSay = ui.easy_say->isChecked();
    bool includeNumbers = ui.numbers->isChecked();
    bool includeUpper = ui.uppercase->isChecked();
    bool includeLower = ui.lowercase->isChecked();
    bool includeSymbols = ui.special_chars->isChecked();
    
    if (ui.all->isChecked()) {
        easyToRead = false;
        easyToSay = false;
    }

    std::string password = generatePassword(length, easyToRead, easyToSay, includeNumbers, includeUpper, includeLower, includeSymbols);

    ui.result->setText(QString::fromStdString(password));
}

void PasswordGenerator::resetSettings()
{
    ui.lenght->setValue(8);
    ui.lenght_slider->setValue(8);
    ui.all->setChecked(true);
    ui.easy_read->setChecked(false);
    ui.easy_say->setChecked(false);
    ui.numbers->setChecked(true);
    ui.uppercase->setChecked(false);
    ui.lowercase->setChecked(true);
    ui.special_chars->setChecked(false);

    updatePassword();
}

void PasswordGenerator::copyPassword()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(ui.result->text());
}