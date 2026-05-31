#pragma once

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Auth
    void on_pushButton_signup_clicked();
    void on_pushButton_login_clicked();
    void on_pushButton_logout_clicked();

    // Person management
    void on_pushButton_addPerson_clicked();
    void on_pushButton_addRelationship_clicked();
    void on_pushButton_removeRelationship_clicked();
    void on_pushButton_deletePerson_clicked();
    void on_pushButton_editRole_clicked();
    void on_pushButton_searchPerson_clicked();

    // Logs
    void on_pushButton_addLog_clicked();
    void on_pushButton_viewLogs_clicked();

    // Analysis
    void on_pushButton_shortestPath_clicked();
    void on_pushButton_clusters_clicked();
    void on_pushButton_topConnected_clicked();
    void on_pushButton_recentActions_clicked();
    void on_pushButton_runDetection_clicked();

    // Undo / Debug
    void on_pushButton_undo_clicked();
    void on_pushButton_toggleDebug_clicked();

    // Network overview
    void on_pushButton_viewNetwork_clicked();

private:
    Ui::MainWindow *ui;

    // Helper to append text to the output area
    void output(const QString &text);
    // Helper to show login state in the UI
    void updateAuthState();
};
