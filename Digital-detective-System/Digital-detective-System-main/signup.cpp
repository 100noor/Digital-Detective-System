#include "signup.h"
#include "ui_signup.h"
#include "dbmanager.h"    // ✅ DB include
#include "logic.h"        // ✅ users vector
#include <QMessageBox>
#include <QRegularExpression>
#include "Login.h"

signup::signup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::signup)
{
    ui->setupUi(this);
}

signup::~signup()
{
    delete ui;
}

void signup::on_signupBtn_clicked()
{
    QString username = ui->usernameInput->text().trimmed();
    QString email    = ui->emailInput->text().trimmed();
    QString password = ui->passwordInput->text();
    QString confirm  = ui->confirmPasswordInput->text();

    // Empty fields
    if(username.isEmpty() || email.isEmpty() ||
        password.isEmpty() || confirm.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Fill all fields");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { background-color:#e74c3c; color:white; }");
        msgBox.exec();
        return;
    }

    // Email validation
    if(!email.contains("@gmail.com"))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Email must contain @gmail.com");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color:#e74c3c; color:white; "
            "padding:5px 15px; border-radius:5px; }");
        msgBox.exec();
        return;
    }

    // Password match
    if(password != confirm)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Passwords do not match");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { background-color:#e74c3c; color:white; }");
        msgBox.exec();
        return;
    }

    // Special character check
    QRegularExpression special("[@#$%^&*!]");
    if(!password.contains(special))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Password must contain a special character (@#$%^&*!)");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { background-color:#e74c3c; color:white; }");
        msgBox.exec();
        return;
    }

    // ✅ Check username already taken
    if(isUsernameTaken(username.toStdString()))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Username already exists!");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { background-color:#e74c3c; color:white; }");
        msgBox.exec();
        return;
    }

    // ✅ Save to DATABASE
    bool dbOk = DatabaseManager::instance().addUser(username, password, "user");

    if(!dbOk)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Database error! Could not save user.");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { background-color:#e74c3c; color:white; }");
        msgBox.exec();
        return;
    }

    // ✅ Also save to memory (so login works in same session)
    User u;
    u.username = username.toStdString();
    u.password = password.toStdString();
    u.role     = "user";
    users.push_back(u);

    // Success
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Success");
    msgBox.setText("Signup Successful! You can now login.");
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { background-color:#2ecc71; color:white; }");
    msgBox.exec();

    // Go to login
    Login *login = new Login();
    login->show();
    this->close();
}

void signup::on_showPasswordCheck_toggled(bool checked)
{
    ui->passwordInput->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password);
}

void signup::on_showPasswordCheck_2_toggled(bool checked)
{
    ui->confirmPasswordInput->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password);
}

void signup::on_loginBtn_clicked()
{
    Login *login = new Login();
    login->show();
    this->close();
}