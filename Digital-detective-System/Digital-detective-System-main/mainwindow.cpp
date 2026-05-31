#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logic.h"

#include <QString>
#include <QMessageBox>

// ─── CONSTRUCTOR / DESTRUCTOR ────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Digital Detective System");
    updateAuthState();

    // Seed random (used internally if needed)
    srand(static_cast<unsigned>(time(nullptr)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─── PRIVATE HELPERS ─────────────────────────────────────────────────────────

void MainWindow::output(const QString &text)
{
    ui->textEdit_output->append(text);
}

void MainWindow::updateAuthState()
{
    bool loggedIn = (currentUser != nullptr);
    // Show current user in status bar
    if (loggedIn)
        statusBar()->showMessage("Logged in as: " + QString::fromStdString(currentUser->username)
                                 + "  [" + QString::fromStdString(currentUser->role) + "]");
    else
        statusBar()->showMessage("Not logged in");

    // Enable/disable the main tab widget based on auth state
    ui->tabWidget_main->setEnabled(loggedIn);
    ui->groupBox_auth->setEnabled(true);   // auth group always accessible
}

// ─── AUTH SLOTS ──────────────────────────────────────────────────────────────

void MainWindow::on_pushButton_signup_clicked()
{
    string username = ui->lineEdit_username->text().trimmed().toStdString();
    string password = ui->lineEdit_password->text().toStdString();

    if (username.empty() || password.empty()) {
        output("ERROR: Username and password cannot be empty.");
        return;
    }
    if (isUsernameTaken(username)) {
        output("ERROR: Username '" + QString::fromStdString(username) + "' already exists.");
        return;
    }
    if (!isValidPassword(password)) {
        output("ERROR: Password must be at least 4 characters.");
        return;
    }
    if (!isValidName(username)) {
        output("ERROR: Invalid username.");
        return;
    }

    User u;
    u.username = username;
    u.password = password;
    u.role     = "user";
    users.push_back(u);
    output("SUCCESS: User '" + QString::fromStdString(username) + "' registered.");
}

void MainWindow::on_pushButton_login_clicked()
{
    string username = ui->lineEdit_username->text().trimmed().toStdString();
    string password = ui->lineEdit_password->text().toStdString();

    for (auto &u : users) {
        if (u.username == username && u.password == password) {
            currentUser = &u;
            output("SUCCESS: Welcome, " + QString::fromStdString(username) + "!");
            updateAuthState();
            return;
        }
    }
    output("ERROR: Invalid username or password.");
}

void MainWindow::on_pushButton_logout_clicked()
{
    if (currentUser) {
        output("Logged out: " + QString::fromStdString(currentUser->username));
        currentUser = nullptr;
        updateAuthState();
    } else {
        output("No user is currently logged in.");
    }
}

// ─── PERSON MANAGEMENT SLOTS ─────────────────────────────────────────────────

void MainWindow::on_pushButton_addPerson_clicked()
{
    string name = ui->lineEdit_name->text().trimmed().toStdString();
    string role = ui->lineEdit_role->text().trimmed().toStdString();

    if (name.empty()) {
        output("ERROR: Name cannot be empty.");
        return;
    }
    if (role.empty()) role = "regular";

    int id = addPerson(name, role, true);
    if (id >= 0)
        output("Person added: " + QString::fromStdString(name)
               + " [" + QString::fromStdString(role) + "]");
    else
        output("ERROR: Failed to add person '" + QString::fromStdString(name) + "'.");
}

void MainWindow::on_pushButton_addRelationship_clicked()
{
    string a = ui->lineEdit_person1->text().trimmed().toStdString();
    string b = ui->lineEdit_person2->text().trimmed().toStdString();

    if (a.empty() || b.empty()) {
        output("ERROR: Both person names are required.");
        return;
    }
    if (a == b) {
        output("ERROR: Cannot create a self-relationship.");
        return;
    }

    addRelationship(a, b);
    output("Relationship added: " + QString::fromStdString(a)
           + " <-> " + QString::fromStdString(b));
}

void MainWindow::on_pushButton_removeRelationship_clicked()
{
    string a = ui->lineEdit_person1->text().trimmed().toStdString();
    string b = ui->lineEdit_person2->text().trimmed().toStdString();

    if (a.empty() || b.empty()) {
        output("ERROR: Both person names are required.");
        return;
    }

    bool ok = removeRelationship(a, b);
    output(ok ? "Relationship removed: " + QString::fromStdString(a)
                    + " <-> " + QString::fromStdString(b)
              : "ERROR: Relationship not found.");
}

void MainWindow::on_pushButton_deletePerson_clicked()
{
    string name = ui->lineEdit_delete->text().trimmed().toStdString();

    if (name.empty()) {
        output("ERROR: Name cannot be empty.");
        return;
    }

    bool ok = deletePersonSoft(name);
    output(ok ? "Person deleted (soft): " + QString::fromStdString(name)
              : "ERROR: Person '" + QString::fromStdString(name) + "' not found.");
}

void MainWindow::on_pushButton_editRole_clicked()
{
    string name = ui->lineEdit_editName->text().trimmed().toStdString();
    string role = ui->lineEdit_editRole->text().trimmed().toStdString();

    if (name.empty() || role.empty()) {
        output("ERROR: Name and new role are required.");
        return;
    }

    int id = findActivePersonByName(name);
    if (id == -1) {
        output("ERROR: Person '" + QString::fromStdString(name) + "' not found.");
        return;
    }

    // Record for undo
    EditRoleRecord er;
    er.personId = id;
    er.oldRole  = people[id].role;
    OperationRecord op;
    op.type    = OP_EDIT_PERSON_ROLE;
    op.editRec = er;
    undoStack.push(op);

    people[id].role = role;
    recentActions.push("Edited Role: " + name + " -> " + role);
    output("Role updated: " + QString::fromStdString(name)
           + " -> " + QString::fromStdString(role));
}

void MainWindow::on_pushButton_searchPerson_clicked()
{
    string name = ui->lineEdit_search->text().trimmed().toStdString();

    if (name.empty()) {
        output("ERROR: Enter a name to search.");
        return;
    }

    int id = findActivePersonByName(name);
    if (id == -1) {
        output("Person not found: " + QString::fromStdString(name));
        return;
    }

    // Build active connection count
    int activeConn = 0;
    for (int nb : adj[id])
        if (nb >= 0 && nb < (int)people.size() && people[nb].active) ++activeConn;

    output("──── Person Details ────");
    output("Name:        " + QString::fromStdString(people[id].name));
    output("Role:        " + QString::fromStdString(people[id].role));
    output("Active:      Yes");
    output("Suspicion:   " + QString::number(people[id].suspicion) + " / 10");
    output("Connections: " + QString::number(activeConn));
    output("────────────────────────");
}

// ─── LOG SLOTS ───────────────────────────────────────────────────────────────

void MainWindow::on_pushButton_addLog_clicked()
{
    string ts      = ui->lineEdit_timestamp->text().trimmed().toStdString();
    string type    = ui->lineEdit_logType->text().trimmed().toStdString();
    string details = ui->lineEdit_logDetails->text().trimmed().toStdString();

    if (ts.empty() || type.empty() || details.empty()) {
        output("ERROR: Timestamp, type, and details are all required.");
        return;
    }
    if (!isValidTimestamp(ts)) {
        output("ERROR: Invalid timestamp. Use YYYY-MM-DD or YYYY-MM-DD HH:MM:SS.");
        return;
    }

    addActivityLog(ts, type, details);
    output("Log added [" + QString::fromStdString(ts) + "]: " + QString::fromStdString(details));
}

void MainWindow::on_pushButton_viewLogs_clicked()
{
    if (logsByDate.empty()) {
        output("No logs recorded yet.");
        return;
    }
    int shiftKey = 3;
    output("──── Activity Logs ────");
    output("Count = " + QString::number(caseCount)); //ADDITION

    for (auto &kv : logsByDate) {
        output("Date: " + QString::fromStdString(kv.first));
        for (auto &e : kv.second) {

            QString decrypted = QString::fromStdString(
                decryptCaesar(e.details, shiftKey)
                );

            output("  [" + QString::fromStdString(e.timestamp) + "] "
                   + QString::fromStdString(e.type) + " - "
                   + decrypted);
        }
    }
    output("───────────────────────");
}

// ─── ANALYSIS SLOTS ──────────────────────────────────────────────────────────

void MainWindow::on_pushButton_shortestPath_clicked()
{
    string a = ui->lineEdit_src->text().trimmed().toStdString();
    string b = ui->lineEdit_dest->text().trimmed().toStdString();

    if (a.empty() || b.empty()) {
        output("ERROR: Source and destination names are required.");
        return;
    }

    int ida = findActivePersonByName(a);
    int idb = findActivePersonByName(b);

    if (ida == -1) { output("ERROR: '" + QString::fromStdString(a) + "' not found."); return; }
    if (idb == -1) { output("ERROR: '" + QString::fromStdString(b) + "' not found."); return; }

    auto path = findShortestPath(ida, idb);
    if (path.empty()) {
        output("No connection found between " + QString::fromStdString(a)
               + " and " + QString::fromStdString(b));
        return;
    }

    QString result;
    for (int i = 0; i < (int)path.size(); ++i) {
        result += QString::fromStdString(people[path[i]].name);
        if (i != (int)path.size() - 1) result += " -> ";
    }
    output("Path: " + result + "  (length: " + QString::number(path.size() - 1) + " hops)");
}

void MainWindow::on_pushButton_clusters_clicked()
{
    auto clusters = getClusters();

    if (clusters.empty()) {
        output("No active people in the network.");
        return;
    }

    output("──── Network Clusters ────");
    for (int i = 0; i < (int)clusters.size(); ++i) {
        output("Cluster " + QString::number(i + 1)
               + "  (" + QString::number(clusters[i].size()) + " members):");
        for (int id : clusters[i]) {
            output("   • " + QString::fromStdString(people[id].name)
                   + " [" + QString::fromStdString(people[id].role) + "]"
                   + " Suspicion: " + QString::number(people[id].suspicion));
        }
    }
    output("──────────────────────────");
}

void MainWindow::on_pushButton_topConnected_clicked()
{
    auto ranked = rankByConnections();
    if (ranked.empty()) {
        output("No active people found.");
        return;
    }

    output("──── Top Connected People ────");
    int limit = min((int)ranked.size(), 10);
    for (int i = 0; i < limit; ++i) {
        output(QString::number(i + 1) + ". "
               + QString::fromStdString(people[ranked[i].second].name)
               + " — " + QString::number(ranked[i].first) + " connections");
    }
    output("──────────────────────────────");
}

void MainWindow::on_pushButton_recentActions_clicked()
{
    if (recentActions.empty()) {
        output("No recent actions recorded.");
        return;
    }

    output("──── Recent Actions (latest first) ────");
    stack<string> temp = recentActions;
    int count = 0;
    while (!temp.empty() && count < 20) {
        output("  " + QString::fromStdString(temp.top()));
        temp.pop();
        ++count;
    }
    output("───────────────────────────────────────");
}

void MainWindow::on_pushButton_runDetection_clicked()
{
    // Reset suspicion scores
    for (auto &p : people) p.suspicion = 0;
    // Re-calculate from all logs
    for (auto &kv : logsByDate)
        for (auto &e : kv.second)
            updateSuspicionFromLog(e.details);

    auto alerts = runRuleEngine();

    output("──── Suspicious Pattern Detection ────");
    if (alerts.empty())
        output("  No suspicious patterns detected.");
    else
        for (auto &a : alerts)
            output("  ⚠  " + QString::fromStdString(a));
    output("──────────────────────────────────────");

    recentActions.push("Ran Suspicious Pattern Detection");
}

// ─── UNDO / DEBUG SLOTS ──────────────────────────────────────────────────────

void MainWindow::on_pushButton_undo_clicked()
{
    bool ok = undoLastOperation();
    output(ok ? "Undo successful." : "Nothing to undo.");
}

void MainWindow::on_pushButton_toggleDebug_clicked()
{
    DEBUG_MODE = !DEBUG_MODE;
    output(QString("Debug mode: ") + (DEBUG_MODE ? "ON" : "OFF"));
}

// ─── NETWORK OVERVIEW SLOT ───────────────────────────────────────────────────

void MainWindow::on_pushButton_viewNetwork_clicked()
{
    bool anyActive = false;
    for (auto &p : people) if (p.active) { anyActive = true; break; }

    if (!anyActive) {
        output("Network is empty.");
        return;
    }

    output("──── Active Network ────");
    for (auto &p : people) {
        if (!p.active) continue;
        QString line = QString::fromStdString(p.name)
                       + " [" + QString::fromStdString(p.role) + "]";
        if (!adj[p.id].empty()) {
            line += "  connected to: ";
            bool first = true;
            for (int nb : adj[p.id]) {
                if (nb >= 0 && nb < (int)people.size() && people[nb].active) {
                    if (!first) line += ", ";
                    line += QString::fromStdString(people[nb].name);
                    first = false;
                }
            }
        }
        output("  " + line);
    }
    output("────────────────────────");
}
