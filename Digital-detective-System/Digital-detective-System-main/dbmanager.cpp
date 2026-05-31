#include "dbmanager.h"
#include "logic.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

// ================= DATABASE CONNECTION =================

bool DatabaseManager::connectToDatabase()
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QODBC");
    }

    QString serverName = "localhost\\SQLEXPRESS03";



    QString dbName = "DigitalDetectiveDB";

    QString dsn = QString(
                      "DRIVER={SQL Server};"
                      "SERVER=%1;"
                      "DATABASE=%2;"
                      "UID=alisha;"
                      "PWD=1234;")
                      .arg(serverName)
                      .arg(dbName);

    db.setDatabaseName(dsn);

    if (!db.open()) {
        qDebug() << "Database connection failed:" << db.lastError().text();

        QString connectionName = "masterConnection";
        if (QSqlDatabase::contains(connectionName))
            QSqlDatabase::removeDatabase(connectionName);

        QSqlDatabase masterDb = QSqlDatabase::addDatabase("QODBC", connectionName);

        QString masterDsn = QString(
                                "DRIVER={SQL Server};"
                                "SERVER=%1;"
                                "DATABASE=master;"
                                "UID=alisha;"
                                "PWD=1234;")
                                .arg(serverName);

        masterDb.setDatabaseName(masterDsn);

        if (masterDb.open()) {
            QSqlQuery createDbQuery(masterDb);
            createDbQuery.exec("IF DB_ID('DigitalDetectiveDB') IS NULL "
                               "CREATE DATABASE DigitalDetectiveDB");
            masterDb.close();
            QSqlDatabase::removeDatabase(connectionName);

            if (!db.open()) {
                qDebug() << "Still failed:" << db.lastError().text();
                return false;
            }
        } else {
            qDebug() << "Master DB connection failed:" << masterDb.lastError().text();
            QSqlDatabase::removeDatabase(connectionName);
            return false;
        }
    }

    qDebug() << "Database connected successfully!";
    return true;
}

// ================= CREATE TABLES =================

bool DatabaseManager::createTables()
{
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    QSqlQuery query(db);

    if (!query.exec("IF OBJECT_ID('Users', 'U') IS NULL "
                    "CREATE TABLE Users ("
                    "id INT PRIMARY KEY IDENTITY(1,1), "
                    "username VARCHAR(255) UNIQUE NOT NULL, "
                    "password VARCHAR(255) NOT NULL, "
                    "role VARCHAR(50) NOT NULL)"))
        qDebug() << "Users table error:" << query.lastError().text();

    if (!query.exec("IF OBJECT_ID('People', 'U') IS NULL "
                    "CREATE TABLE People ("
                    "id INT PRIMARY KEY IDENTITY(0,1), "
                    "name VARCHAR(255) UNIQUE NOT NULL, "
                    "role VARCHAR(50) NOT NULL, "
                    "active BIT NOT NULL, "
                    "suspicion INT NOT NULL)"))
        qDebug() << "People table error:" << query.lastError().text();

    if (!query.exec("IF OBJECT_ID('Relationships', 'U') IS NULL "
                    "CREATE TABLE Relationships ("
                    "person1_id INT NOT NULL, "
                    "person2_id INT NOT NULL, "
                    "PRIMARY KEY(person1_id, person2_id), "
                    "FOREIGN KEY(person1_id) REFERENCES People(id), "
                    "FOREIGN KEY(person2_id) REFERENCES People(id))"))
        qDebug() << "Relationships table error:" << query.lastError().text();

    if (!query.exec("IF OBJECT_ID('Logs', 'U') IS NULL "
                    "CREATE TABLE Logs ("
                    "id INT PRIMARY KEY IDENTITY(1,1), "
                    "log_date VARCHAR(20) NOT NULL, "
                    "timestamp VARCHAR(50) NOT NULL, "
                    "log_type VARCHAR(50) NOT NULL, "
                    "details VARCHAR(MAX) NOT NULL)"))
        qDebug() << "Logs table error:" << query.lastError().text();

    qDebug() << "Tables created successfully!";
    return true;
}

// ================= USERS =================

bool DatabaseManager::addUser(const QString& username,
                              const QString& password,
                              const QString& role)
{
    QSqlQuery check(db);
    check.prepare("SELECT username FROM Users WHERE username = ?");
    check.addBindValue(username);

    if (check.exec() && check.next()) {
        qDebug() << "User already exists";
        return false;
    }

    // ============ HASH PASSWORD BEFORE SAVE
    string hashed = hashPassword(password.toStdString());

    QSqlQuery query(db);
    query.prepare("INSERT INTO Users(username, password, role) VALUES(?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(QString::fromStdString(hashed));
    query.addBindValue(role);

    return query.exec();
}

//----------------- verifyUser-------------------------

bool DatabaseManager::verifyUser(const QString& username,
                                 const QString& password,
                                 QString& outRole)
{
    //  HASH INPUT PASSWORD
    string hashed = hashPassword(password.toStdString());

    QSqlQuery query(db);
    query.prepare("SELECT role FROM Users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(QString::fromStdString(hashed));

    if (query.exec() && query.next()) {
        outRole = query.value(0).toString();
        return true;
    }

    return false;
}

bool DatabaseManager::loadAllUsers(std::vector<User>& usersList)
{
    QSqlQuery query(db);
    if (!query.exec("SELECT username, password, role FROM Users"))
        return false;

    usersList.clear();
    while (query.next()) {
        User u;
        u.username = query.value(0).toString().toStdString();
        u.password = query.value(1).toString().toStdString();
        u.role     = query.value(2).toString().toStdString();
        usersList.push_back(u);
    }
    return true;
}

// ================= PEOPLE =================

int DatabaseManager::addPerson(const QString& name, const QString& role, bool active, int suspicion)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO People(name, role, active, suspicion) VALUES(?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(role);
    query.addBindValue(active);
    query.addBindValue(suspicion);

    if (query.exec()) {
        QSqlQuery idQuery(db);
        idQuery.exec("SELECT SCOPE_IDENTITY()");
        if (idQuery.next())
            return idQuery.value(0).toInt();
    }
    return -1;
}

bool DatabaseManager::updatePersonActiveStatus(int id, bool active)
{
    QSqlQuery query(db);
    query.prepare("UPDATE People SET active = ? WHERE id = ?");
    query.addBindValue(active);
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::updatePersonRole(int id, const QString& role)
{
    QSqlQuery query(db);
    query.prepare("UPDATE People SET role = ? WHERE id = ?");
    query.addBindValue(role);
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::updatePersonSuspicion(int id, int suspicion)
{
    QSqlQuery query(db);
    query.prepare("UPDATE People SET suspicion = ? WHERE id = ?");
    query.addBindValue(suspicion);
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::loadAllPeople(std::vector<Person>& peopleList, std::map<std::string, int>& nameToId)
{
    QSqlQuery query(db);
    if (!query.exec("SELECT id, name, role, active, suspicion FROM People ORDER BY id ASC"))
        return false;

    peopleList.clear();
    nameToId.clear();

    while (query.next()) {
        Person p;
        p.id        = query.value(0).toInt();
        p.name      = query.value(1).toString().toStdString();
        p.role      = query.value(2).toString().toStdString();
        p.active    = query.value(3).toBool();
        p.suspicion = query.value(4).toInt();
        peopleList.push_back(p);
        nameToId[p.name] = p.id;
    }
    return true;
}

// ================= RELATIONSHIPS =================

bool DatabaseManager::addRelationship(int person1Id, int person2Id)
{
    int p1 = std::min(person1Id, person2Id);
    int p2 = std::max(person1Id, person2Id);

    QSqlQuery query(db);
    query.prepare("INSERT INTO Relationships(person1_id, person2_id) VALUES(?, ?)");
    query.addBindValue(p1);
    query.addBindValue(p2);
    return query.exec();
}

bool DatabaseManager::removeRelationship(int person1Id, int person2Id)
{
    int p1 = std::min(person1Id, person2Id);
    int p2 = std::max(person1Id, person2Id);

    QSqlQuery query(db);
    query.prepare("DELETE FROM Relationships WHERE person1_id = ? AND person2_id = ?");
    query.addBindValue(p1);
    query.addBindValue(p2);
    return query.exec();
}

bool DatabaseManager::loadAllRelationships(std::vector<std::vector<int>>& adjList, int totalPeopleCount)
{
    adjList.assign(totalPeopleCount, std::vector<int>());

    QSqlQuery query(db);
    if (!query.exec("SELECT person1_id, person2_id FROM Relationships"))
        return false;

    while (query.next()) {
        int p1 = query.value(0).toInt();
        int p2 = query.value(1).toInt();
        if (p1 < totalPeopleCount && p2 < totalPeopleCount) {
            adjList[p1].push_back(p2);
            adjList[p2].push_back(p1);
        }
    }
    return true;
}

// ================= LOGS =================

bool DatabaseManager::addLog(const QString& timestamp, const QString& type, const QString& details)
{
    QString date = timestamp.left(10);
    QSqlQuery query(db);
    query.prepare("INSERT INTO Logs(log_date, timestamp, log_type, details) VALUES(?, ?, ?, ?)");
    query.addBindValue(date);
    query.addBindValue(timestamp);
    query.addBindValue(type);
    query.addBindValue(details);
    return query.exec();
}

bool DatabaseManager::loadAllLogs(std::map<std::string, std::vector<LogEntry>>& logsByDate)
{
    logsByDate.clear();
    QSqlQuery query(db);
    if (!query.exec("SELECT log_date, timestamp, log_type, details FROM Logs ORDER BY timestamp ASC"))
        return false;

    while (query.next()) {
        std::string date = query.value(0).toString().toStdString();
        LogEntry e;
        e.timestamp = query.value(1).toString().toStdString();
        e.type      = query.value(2).toString().toStdString();
        e.details   = query.value(3).toString().toStdString();
        logsByDate[date].push_back(e);
    }
    return true;
}