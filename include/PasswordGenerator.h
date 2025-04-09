#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PasswordGenerator.h"
#include "generator.h"

class PasswordGenerator : public QMainWindow
{
    Q_OBJECT

public:
    PasswordGenerator(QWidget* parent = nullptr);
    ~PasswordGenerator();

private:
    Ui::PasswordGeneratorClass ui;

    void updatePassword();
    void resetSettings();
    void copyPassword();

};
