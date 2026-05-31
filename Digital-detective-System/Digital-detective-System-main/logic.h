#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <regex>


using namespace std;
#include <string>
using std::string;

string hashPassword(const string &password);

// ─── DATA STRUCTURES ────────────────────────────────────────────────────────

struct User {
    string username;
    string password;
    string role;
};

struct Person {
    int    id;
    string name;
    string role;
    bool   active;
    int    suspicion;
};

struct LogEntry {
    string timestamp;
    string type;
    string details;
};

enum OpType { OP_DELETE_PERSON, OP_ADD_LOG, OP_EDIT_PERSON_ROLE };

struct DeleteRecord {
    int          personId;
    vector<int>  removedEdges;
};

struct AddLogRecord {
    string    timestamp;
    size_t    indexInDateVector;
    LogEntry  entry;
};

struct EditRoleRecord {
    int    personId;
    string oldRole;
};

struct OperationRecord {
    OpType        type;
    DeleteRecord  delRec;
    AddLogRecord  logRec;
    EditRoleRecord editRec;
};

// ─── GLOBALS ─────────────────────────────────────────────────────────────────

extern bool DEBUG_MODE;

extern vector<User>              users;
extern User*                     currentUser;
extern vector<Person>            people;
extern map<string, int>          name_to_id;
extern vector<vector<int>>       adj;
extern map<string, vector<LogEntry>> logsByDate;
extern stack<string>             recentActions;
extern stack<OperationRecord>    undoStack;
extern int caseCount;      // Global-Changes

// ─── INIT ────────────────────────────────────────────────────────────────────
void initializeSystem();

// ─── HELPERS ─────────────────────────────────────────────────────────────────

string toLowerCopy(const string &s);
bool   containsWordInsensitive(const string &text, const string &token);
bool   isValidName(const string &name);
bool   isValidTimestamp(const string &ts);
bool   isUsernameTaken(const string &username);
bool   isValidPassword(const string &pwd);
string datePart(const string &timestamp);
void   debugPrint(const string &msg);

// ─── AUTH ────────────────────────────────────────────────────────────────────

void signUp();
bool login();
void logout();

// ─── PERSON / GRAPH ──────────────────────────────────────────────────────────

int  findActivePersonByName(const string &name);
int  addPerson(const string &name, const string &role, bool setRoleOnReactivate = false);
void addRelationship(const string &a, const string &b);
bool removeRelationship(const string &a, const string &b);
bool deletePersonSoft(const string &name);
void printPerson(int id);

// ─── LOGS ────────────────────────────────────────────────────────────────────

void addActivityLog(const string &timestamp, const string &type, const string &details);

// ─── UNDO ────────────────────────────────────────────────────────────────────

bool undoLastOperation();

// ─── ANALYSIS ────────────────────────────────────────────────────────────────

vector<int>          findShortestPath(int src, int dest);
vector<vector<int>>  getClusters();
vector<pair<int,int>> rankByConnections();
void                 updateSuspicionFromLog(const string &details);
vector<string>       runRuleEngine();
void                 runSuspiciousDetection();
void traverseAllEvidence();        //TRAVERSE-Addition
// Encryption
string encryptCaesar(string text, int shift);
string decryptCaesar(string text, int shift);

// Word learning
extern map<string, int>  suspiciousWordFrequency;
extern vector<string>    learnedSuspiciousWords;
extern vector<string>    commonWords;
void learnSuspiciousWords(const string &details);
vector<string> extractWords(string text);
bool isCommonWord(const string &word);