#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PasswordGenerator.h"
#include <QFuture>
#include <QTimer>
#include <QElapsedTimer>

struct PasswordParams {
    int length;
    bool easyToRead;
    bool easyToSay;
    bool includeNumbers;
    bool includeUpper;
    bool includeLower;
    bool includeSymbols;
    std::string excludedChars;
};

struct PasswordDetails {
    bool hasDigits = false;
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasSpecialChars = false;
    bool hasSequentialChars = false;
    bool hasRepeatedChars = false;
    int length = 0;
    int uniqueChars = 0;
};

class PasswordGenerator : public QMainWindow
{
    Q_OBJECT

public:
    PasswordGenerator(QWidget* parent = nullptr);
    ~PasswordGenerator();

private slots:
    void schedulePasswordUpdate();
    void generatePasswordSafely();
    void applyGeneratedPassword(const QString& password);
    void handlePasswordError(const QString& errorMsg);
    void resetSettings();
    void copyPassword();
    void finishGeneration();
    void updateLengthUI(int value);

private:
    Ui::PasswordGeneratorClass ui;

    int evaluatePasswordStrength(const std::string& password);
    void updatePasswordStrengthBar(int strengthLevel);
    PasswordDetails evaluatePasswordDetails();
    QString getPasswordImprovementTips(const PasswordDetails& details);

    void setUIEnabled(bool enabled);
    bool m_updatePending;
    bool m_passwordGeneration;
    QTimer* m_debounceTimer;
    QFuture<void> m_updateFuture;
};