#pragma once

#include <QSqlDatabase>

#include <QSqlQuery>

#include <QSqlError>

#include <QString>

#include <QDebug>

#include <vector>

#include <map>

struct User;

struct Person;

struct LogEntry;

class DatabaseManager

{

public:

    static DatabaseManager& instance()

    {

        static DatabaseManager instance;

        return instance;

    }



    bool connectToDatabase();

    bool createTables();



    // User operations

    bool addUser(const QString& username, const QString& password, const QString& role);

    bool verifyUser(const QString& username, const QString& password, QString& outRole);

    bool loadAllUsers(std::vector<User>& usersList);



    // Person operations

    int addPerson(const QString& name, const QString& role, bool active, int suspicion);

    bool updatePersonActiveStatus(int id, bool active);

    bool updatePersonRole(int id, const QString& role);

    bool updatePersonSuspicion(int id, int suspicion);

    bool loadAllPeople(std::vector<Person>& peopleList, std::map<std::string, int>& nameToId);



    // Relationship operations

    bool addRelationship(int person1Id, int person2Id);

    bool removeRelationship(int person1Id, int person2Id);

    bool loadAllRelationships(std::vector<std::vector<int>>& adjList, int totalPeopleCount);



    // Log operations

    bool addLog(const QString& timestamp, const QString& type, const QString& details);

    bool loadAllLogs(std::map<std::string, std::vector<LogEntry>>& logsByDate);

private:

    DatabaseManager() {}

    DatabaseManager(const DatabaseManager&) = delete;

    DatabaseManager& operator=(const DatabaseManager&) = delete;



    QSqlDatabase db;

};