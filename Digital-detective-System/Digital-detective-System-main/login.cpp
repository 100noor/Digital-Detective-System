#include "login.h"
#include "ui_login.h"
#include "signup.h"
#include "dbmanager.h"
#include "logic.h"
#include <QMessageBox>
#include "mainwindow.h"

Login::Login(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}
void Login::on_loginBtn_clicked()
{
    QString username = ui->usernameInput->text().trimmed();
    QString password = ui->passwordInput->text();

    if(username.isEmpty() || password.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Fill all fields");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color:#3498db; color:white; "
            "padding:5px 15px; border-radius:5px; }");
        msgBox.exec();
        return;
    }

    // ✅ Verify from DATABASE
    QString role;
    bool ok = DatabaseManager::instance().verifyUser(username, password, role);

    if(!ok)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Invalid username or password!");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color:#e74c3c; color:white; "
            "padding:5px 15px; border-radius:5px; }");
        msgBox.exec();
        return;
    }

    // ✅ Set currentUser from memory (find in users vector)
    for(auto &u : users)
    {
        if(u.username == username.toStdString())
        {
            currentUser = &u;
            break;
        }
    }

    // Success
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Success");
    msgBox.setText("Login Successful! Welcome, " + username);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; font-size: 14px; }"
        "QPushButton { background-color:#2ecc71; color:white; "
        "padding:5px 15px; border-radius:5px; }");
    msgBox.exec();

    MainWindow *obj = new MainWindow();
    obj->show();
    this->close();
}

void Login::on_signupBtn_clicked()
{
    signup *s = new signup();
    s->show();
    this->close();
}

void Login::on_checkBox_toggled(bool checked)
{
    ui->passwordInput->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password);
}