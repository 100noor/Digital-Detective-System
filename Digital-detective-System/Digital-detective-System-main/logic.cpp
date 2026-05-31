#include "logic.h"
#include "dbmanager.h"

// ─── GLOBALS ─────────────────────────────────────────────────────────────────
bool DEBUG_MODE = false;
bool DB_LOADING = false;

vector<User>                 users;
User*                        currentUser = nullptr;
vector<Person>               people;
map<string, int>             name_to_id;
vector<vector<int>>          adj;
map<string, vector<LogEntry>> logsByDate;
stack<string>                recentActions;
stack<OperationRecord>       undoStack;

int caseCount = 0;

//ADDITION

map<string, int>             suspiciousWordFrequency;
vector<string>               learnedSuspiciousWords;
vector<string>               commonWords = {
    "the", "is", "was", "and", "are", "hi", "for",
    "hello", "today", "meeting", "am",
    "person", "normal", "account", "from", "casual"
};
// ─── HASHHH────────────────────────────────────────────────────────────────────
std::string hashPassword(const std::string &password);
#include <QCryptographicHash>

string hashPassword(const string &password) {
    QByteArray data = QByteArray::fromStdString(password);

    QByteArray hashed = QCryptographicHash::hash(
        data,
        QCryptographicHash::Sha256
        );

    return hashed.toHex().toStdString();
}
// ─── INIT ────────────────────────────────────────────────────────────────────

void initializeSystem() {
    DatabaseManager& db = DatabaseManager::instance();
    if (db.connectToDatabase()) {
        db.createTables();
        db.loadAllUsers(users);
        db.loadAllPeople(people, name_to_id);
        db.loadAllRelationships(adj, people.size());
        db.loadAllLogs(logsByDate);
        caseCount = 0;
        for (auto &kv : logsByDate)
            caseCount += kv.second.size();

        debugPrint("Database initialized successfully.");
    }
}

// ─── HELPERS ─────────────────────────────────────────────────────────────────

string toLowerCopy(const string &s) {
    string out = s;
    for (char &c : out)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return out;
}

bool containsWordInsensitive(const string &text, const string &token) {
    if (token.empty()) return false;
    string t = toLowerCopy(text);
    string k = toLowerCopy(token);
    size_t pos = 0;
    while ((pos = t.find(k, pos)) != string::npos) {
        bool left_ok  = (pos == 0) || !isalpha(static_cast<unsigned char>(t[pos - 1]));
        bool right_ok = ((pos + k.size()) >= t.size()) ||
                        !isalpha(static_cast<unsigned char>(t[pos + k.size()]));
        if (left_ok && right_ok) return true;
        pos += 1;
    }
    return false;
}

bool isValidName(const string &name) {
    if (name.empty()) return false;
    bool anyNonSpace = false;
    for (char c : name)
        if (!isspace(static_cast<unsigned char>(c))) { anyNonSpace = true; break; }
    if (!anyNonSpace) return false;
    for (char c : name)
        if (iscntrl(static_cast<unsigned char>(c))) return false;
    return true;
}

bool isValidTimestamp(const string &ts) {
    static const regex reDateTime(R"(^\d{4}-\d{2}-\d{2}(?:\s+\d{2}:\d{2}:\d{2})?$)");
    return regex_match(ts, reDateTime);
}

bool isUsernameTaken(const string &username) {
    for (auto &u : users)
        if (u.username == username) return true;
    return false;
}

bool isValidPassword(const string &pwd) {
    return pwd.length() >= 4;
}

string datePart(const string &timestamp) {
    if (timestamp.size() >= 10) return timestamp.substr(0, 10);
    return timestamp;
}

void debugPrint(const string &msg) {
    if (DEBUG_MODE)
        cout << "[DEBUG] " << msg << endl;
}

// ─── AUTH ────────────────────────────────────────────────────────────────────

void signUp() {
    string username, password;
    cout << "\n=== SIGN UP ===\n";
    cout << "Enter username: ";
    getline(cin, username);
    cout << "Enter password: ";
    getline(cin, password);

    if (isUsernameTaken(username)) {
        cout << "Username already exists!\n";
        return;
    }
    if (!isValidPassword(password)) {
        cout << "Password too short (minimum 4 characters)!\n";
        return;
    }
    if (!isValidName(username)) {
        cout << "Invalid username!\n";
        return;
    }

    User u;
    u.username = username;
    u.password = hashPassword(password);
    u.role     = "user";
    users.push_back(u);
    DatabaseManager::instance().addUser(QString::fromStdString(username), QString::fromStdString(password), "user");
    cout << "Signup successful!\n";
}

bool login() {
    string username, password;
    cout << "\n=== LOGIN ===\n";
    cout << "Enter username: ";
    getline(cin, username);
    cout << "Enter password: ";
    getline(cin, password);

    for (auto &u : users) {
        if (u.username == username && u.password == password) {
            currentUser = &u;
            cout << "Login successful! Welcome, " << username << "\n";
            return true;
        }
    }
    cout << "Invalid username or password!\n";
    return false;
}

void logout() {
    if (currentUser) {
        cout << "User " << currentUser->username << " logged out.\n";
        currentUser = nullptr;
    }
}

// ─── PERSON / GRAPH ──────────────────────────────────────────────────────────

int findActivePersonByName(const string &name) {
    auto it = name_to_id.find(name);
    if (it == name_to_id.end()) return -1;
    int id = it->second;
    if (id < 0 || id >= (int)people.size()) return -1;
    if (!people[id].active) return -1;
    return id;
}

int addPerson(const string &name, const string &role, bool setRoleOnReactivate) {
    if (!isValidName(name)) return -1;

    auto it = name_to_id.find(name);
    if (it != name_to_id.end()) {
        int id = it->second;
        if (!people[id].active) {
            people[id].active = true;
            if (setRoleOnReactivate && !role.empty())
                people[id].role = role;
            recentActions.push("Reactivated Person: " + name);
        }
        return id;
    }

    int id = (int)people.size();
    Person p;
    p.id        = id;
    p.name      = name;
    p.role      = role;
    p.active    = true;
    p.suspicion = 0;
    people.push_back(p);
    name_to_id[name] = id;
    adj.emplace_back();
    DatabaseManager::instance().addPerson(QString::fromStdString(name), QString::fromStdString(role), true, 0);
    recentActions.push("Added Person: " + name);
    debugPrint("Adding person: " + name);
    return id;
}

void addRelationship(const string &a, const string &b) {
    if (!isValidName(a) || !isValidName(b)) {
        recentActions.push("Failed Relationship Add (invalid name)");
        return;
    }
    if (a == b) {
        recentActions.push("Prevented self-relationship for: " + a);
        return;
    }

    int ida = findActivePersonByName(a);
    int idb = findActivePersonByName(b);
    if (ida == -1) ida = addPerson(a, "regular", false);
    if (idb == -1) idb = addPerson(b, "regular", false);
    if (ida == -1 || idb == -1) return;

    vector<int> &va = adj[ida];
    vector<int> &vb = adj[idb];
    if (find(va.begin(), va.end(), idb) == va.end()) va.push_back(idb);
    if (find(vb.begin(), vb.end(), ida) == vb.end()) vb.push_back(ida);
    DatabaseManager::instance().addRelationship(ida, idb);
    recentActions.push("Added Relationship: " + a + " <-> " + b);
    debugPrint("Creating relationship between " + a + " and " + b);
}

bool removeRelationship(const string &a, const string &b) {
    int ida = findActivePersonByName(a);
    int idb = findActivePersonByName(b);
    if (ida == -1 || idb == -1) return false;

    bool removed = false;
    auto &va = adj[ida];
    auto &vb = adj[idb];

    auto it = remove(va.begin(), va.end(), idb);
    if (it != va.end()) { va.erase(it, va.end()); removed = true; }

    it = remove(vb.begin(), vb.end(), ida);
    if (it != vb.end()) { vb.erase(it, vb.end()); removed = true; }

    if (removed) {
        DatabaseManager::instance().removeRelationship(ida, idb);
        recentActions.push("Removed Relationship: " + a + " <-> " + b);
    }
    return removed;
}

bool deletePersonSoft(const string &name) {
    int id = findActivePersonByName(name);
    if (id == -1) return false;

    DeleteRecord rec;
    rec.personId     = id;
    rec.removedEdges = adj[id];

    for (int nb : adj[id]) {
        auto &v = adj[nb];
        v.erase(remove(v.begin(), v.end(), id), v.end());
    }
    adj[id].clear();
    people[id].active = false;
    DatabaseManager::instance().updatePersonActiveStatus(id, false);
    for (int nb : rec.removedEdges) {
        DatabaseManager::instance().removeRelationship(id, nb);
    }
    recentActions.push("Deleted Person (soft): " + name);

    OperationRecord op;
    op.type   = OP_DELETE_PERSON;
    op.delRec = rec;
    undoStack.push(op);
    return true;
}

void printPerson(int id) {
    if (id < 0 || id >= (int)people.size()) {
        cout << "Invalid person id\n";
        return;
    }
    int activeConnections = 0;
    for (int nb : adj[id])
        if (nb >= 0 && nb < (int)people.size() && people[nb].active)
            ++activeConnections;

    cout << "Name: "           << people[id].name
         << " | Role: "        << people[id].role
         << " | Active: "      << (people[id].active ? "Yes" : "No")
         << " | Suspicion: "   << people[id].suspicion
         << " | Connections: " << activeConnections << "\n";
}

// ─── ENCRYPTION ──────────────────────────────────────────────────────────────

string encryptCaesar(string text, int shift) {
    shift = shift % 26;
    for (char &c : text) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = (c - base + shift) % 26 + base;
        }
    }
    return text;
}

// ─── DECRYPTION ──────────────────────────────────────────────────────────────

string decryptCaesar(string text, int shift) {
    shift = shift % 26;
    for (char &c : text) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = (c - base - shift + 26) % 26 + base;
        }
    }
    return text;
}

// ─── WORD LEARNING ───────────────────────────────────────────────────────────

vector<string> extractWords(string text) {
    vector<string> words;
    string word = "";
    for (char c : text) {
        if (isalpha(c)) word += tolower(c);
        else { if (!word.empty()) { words.push_back(word); word = ""; } }
    }
    if (!word.empty()) words.push_back(word);
    return words;
}

bool isCommonWord(const string &word) {
    return find(commonWords.begin(), commonWords.end(), word) != commonWords.end();
}

void learnSuspiciousWords(const string &details) {
    vector<string> words = extractWords(details);
    for (string word : words) {
        if (word.length() <= 3) continue;
        if (isCommonWord(word)) continue;
        suspiciousWordFrequency[word]++;
        if (suspiciousWordFrequency[word] >= 3) {
            if (find(learnedSuspiciousWords.begin(),
                     learnedSuspiciousWords.end(), word)
                == learnedSuspiciousWords.end()) {
                learnedSuspiciousWords.push_back(word);
                recentActions.push("Learned suspicious word: " + word);
            }
        }
    }
}

// ─── LOGS ────────────────────────────────────────────────────────────────────

void addActivityLog(const string &timestamp, const string &type, const string &details) {
    if (!isValidTimestamp(timestamp)) {
        recentActions.push("Rejected Log (invalid timestamp)");
        return;
    }

    extern bool DB_LOADING;

    string date = datePart(timestamp);
    caseCount++;

    LogEntry e;
    e.timestamp = timestamp;
    e.type = type;
    e.details = encryptCaesar(details, 3);

    if (!DB_LOADING) {
        DatabaseManager::instance().addLog(
            QString::fromStdString(timestamp),
            QString::fromStdString(type),
            QString::fromStdString(details)
            );
    }

    recentActions.push("Added Log (" + timestamp + "): " + details);

    AddLogRecord lr;
    lr.timestamp = timestamp;
    lr.entry = e;

    OperationRecord op;
    op.type = OP_ADD_LOG;
    op.logRec = lr;
    undoStack.push(op);
}

// ─── UNDO ────────────────────────────────────────────────────────────────────

bool undoLastOperation() {
    if (undoStack.empty()) return false;
    OperationRecord op = undoStack.top();
    undoStack.pop();

    if (op.type == OP_DELETE_PERSON) {
        int id = op.delRec.personId;
        if (id < 0 || id >= (int)people.size()) return false;
        people[id].active = true;
        for (int nb : op.delRec.removedEdges) {
            if (nb >= 0 && nb < (int)people.size() && people[nb].active) {
                auto &v1 = adj[id];
                auto &v2 = adj[nb];
                if (find(v1.begin(), v1.end(), nb) == v1.end()) v1.push_back(nb);
                if (find(v2.begin(), v2.end(), id) == v2.end()) v2.push_back(id);
            }
        }
        recentActions.push("Undo Delete: " + people[id].name);
        return true;
    }

    if (op.type == OP_ADD_LOG) {
        string date = datePart(op.logRec.timestamp);
        auto   it   = logsByDate.find(date);
        if (it != logsByDate.end()) {
            auto &vec = it->second;
            for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
                if (iter->timestamp == op.logRec.entry.timestamp &&
                    iter->type      == op.logRec.entry.type      &&
                    iter->details   == op.logRec.entry.details) {
                    vec.erase(iter);
                    recentActions.push("Undo Add Log: " + op.logRec.entry.timestamp
                                       + " : " + op.logRec.entry.details);
                    if (vec.empty()) logsByDate.erase(date);
                    return true;
                }
            }
        }
        return false;
    }

    if (op.type == OP_EDIT_PERSON_ROLE) {
        int id = op.editRec.personId;
        if (id < 0 || id >= (int)people.size()) return false;
        people[id].role = op.editRec.oldRole;
        recentActions.push("Undo Edit Role: " + people[id].name);
        return true;
    }

    return false;
}

// ─── ANALYSIS ────────────────────────────────────────────────────────────────

vector<int> findShortestPath(int src, int dest) {
    if (src < 0 || dest < 0 ||
        src >= (int)people.size() || dest >= (int)people.size()) return {};
    if (!people[src].active || !people[dest].active) return {};

    vector<int>  prev(people.size(), -1);
    vector<char> vis(people.size(), 0);
    queue<int>   q;
    vis[src] = 1;
    q.push(src);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        debugPrint("Visiting node: " + people[u].name);
        if (u == dest) break;
        for (int v : adj[u]) {
            if (!vis[v] && people[v].active) {
                vis[v]  = 1;
                prev[v] = u;
                q.push(v);
            }
        }
    }

    if (!vis[dest]) return {};
    vector<int> path;
    for (int at = dest; at != -1; at = prev[at])
        path.push_back(at);
    reverse(path.begin(), path.end());
    return path;
}

vector<vector<int>> getClusters() {
    int n = (int)people.size();
    vector<char>        vis(n, 0);
    vector<vector<int>> clusters;

    for (int i = 0; i < n; ++i) {
        if (!people[i].active || vis[i]) continue;
        vector<int> comp;
        queue<int>  q;
        q.push(i); vis[i] = 1;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            comp.push_back(u);
            for (int v : adj[u])
                if (people[v].active && !vis[v]) { vis[v] = 1; q.push(v); }
        }
        clusters.push_back(comp);
    }
    return clusters;
}

vector<pair<int,int>> rankByConnections() {
    vector<pair<int,int>> list;
    for (size_t i = 0; i < people.size(); ++i) {
        if (!people[i].active) continue;
        int cnt = 0;
        for (int nb : adj[i])
            if (nb >= 0 && nb < (int)people.size() && people[nb].active) ++cnt;
        list.push_back({cnt, (int)i});
    }
    sort(list.rbegin(), list.rend());
    return list;
}

static const vector<string> SUSP_KWS     = {"transfer","money","drugs","hack","suspicious"};
static const vector<int>    SUSP_WEIGHTS = {3, 2, 4, 4, 2};

void updateSuspicionFromLog(const string &details) {
    if (details.empty()) return;
    string lowerDetails = toLowerCopy(details);
    bool   anyHit       = false;

    for (size_t k = 0; k < SUSP_KWS.size(); ++k) {
        if (containsWordInsensitive(lowerDetails, SUSP_KWS[k])) {
            anyHit = true;
            for (auto &p : people) {
                if (!p.active) continue;
                if (containsWordInsensitive(lowerDetails, p.name)) {
                    p.suspicion += SUSP_WEIGHTS[k];
                    if (p.suspicion > 10) p.suspicion = 10;
                    DatabaseManager::instance().updatePersonSuspicion(p.id, p.suspicion);
                    recentActions.push("Updated suspicion for " + p.name);
                }
            }
        }
    }

    if (!anyHit) {
        for (auto &p : people) {
            if (!p.active) continue;
            if (containsWordInsensitive(lowerDetails, p.name)) {
                p.suspicion += 1;
                if (p.suspicion > 10) p.suspicion = 10;
                DatabaseManager::instance().updatePersonSuspicion(p.id, p.suspicion);
                recentActions.push("Updated suspicion (name found) for " + p.name);
            }
        }
    }
}

vector<string> runRuleEngine() {
    vector<string> alerts;

    for (auto &p : people) {
        if (!p.active) continue;
        int suspectsConnected = 0;
        int myId = p.id;
        for (int nb : adj[myId])
            if (nb >= 0 && nb < (int)people.size() && people[nb].active &&
                people[nb].role == "suspect")
                ++suspectsConnected;

        if (p.suspicion >= 8 || suspectsConnected > 5)
            alerts.push_back("HIGH RISK: " + p.name);
        else if (p.suspicion >= 5)
            alerts.push_back("MEDIUM RISK: " + p.name);
    }

    auto clusters = getClusters();
    for (size_t i = 0; i < clusters.size(); ++i) {
        int suspectCount = 0;
        for (int j : clusters[i])
            if (people[j].role == "suspect") ++suspectCount;
        if (suspectCount >= (int)clusters[i].size() / 2 && suspectCount >= 2)
            alerts.push_back("SUSPICIOUS CLUSTER #" + to_string(i + 1));
    }

    for (auto &kv : logsByDate)
        if ((int)kv.second.size() >= 5)
            alerts.push_back("HIGH LOG VOLUME: " + kv.first);

    return alerts;
}

void runSuspiciousDetection() {
    for (auto &p : people) {
        p.suspicion = 0;
        DatabaseManager::instance().updatePersonSuspicion(p.id, 0);
    }
    for (auto &kv : logsByDate)
        for (auto &e : kv.second)
            updateSuspicionFromLog(e.details);

    auto alerts = runRuleEngine();
    if (alerts.empty())
        cout << "No suspicious patterns detected.\n";
    else {
        cout << "ALERTS FOUND:\n";
        for (auto &a : alerts) cout << "  - " << a << "\n";
    }
    recentActions.push("Ran Suspicious Pattern Detection");
}
void traverseAllEvidence() {
    cout << "\n=== ALL EVIDENCE/CLUES ===\n";


    for (auto &kv : logsByDate) {
        cout << "Date: " << kv.first << "\n";
        for (auto &entry : kv.second) {
            cout << "  [" << entry.type << "] "
                 << entry.timestamp << " — "
                 << entry.details << "\n";
        }
    }
    cout << "\n==========================\n";
    cout << "Total Case Count: " << caseCount << "\n\n";
}