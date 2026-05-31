#ifndef SIGNUP_H
#define SIGNUP_H

#include <QWidget>

namespace Ui {
class signup;
}

class signup : public QWidget
{
    Q_OBJECT

public:
    explicit signup(QWidget *parent = nullptr);
    ~signup();

private slots:
    void on_signupBtn_clicked();

    void on_showPasswordCheck_toggled(bool checked);

    void on_showPasswordCheck_2_toggled(bool checked);

    void on_loginBtn_clicked();

private:
    Ui::signup *ui;
};

#endif // SIGNUP_H

