#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdlib> 

using namespace std;

const int MAX_USERS = 100;
const int MAX_FOLLOWERS = 100;
const int MAX_REPOSITORIES = 100;
const int MAX_COMMITS = 100;
const int MAX_FILES = 100;

class Commit {
private:
    string message;
    Commit* next;
public:
    Commit(string msg) : message(msg), next(nullptr) {}
    string getMessage() const { return message; }
    Commit* getNext() const { return next; }
    void setNext(Commit* nextCommit) { next = nextCommit; }
};

class File {
private:
    string name;
    File* next;
public:
    File(string filename) : name(filename), next(nullptr) {}
    string getName() const { return name; }
    File* getNext() const { return next; }
    void setNext(File* nextFile) { next = nextFile; }
};

class Repository {
private:
    string name;
    Commit* commits[MAX_COMMITS];
    File* files[MAX_FILES];
    int commitCount;
    int fileCount;
    bool isPublic;
    int forkCount;


public:
    Repository(string repoName, bool public_) : name(repoName), isPublic(public_), commitCount(0), fileCount(0), forkCount(0) {}
    string getName() const { return name; }

    bool addCommit(Commit* commit) {
        if (commitCount < MAX_COMMITS) {
            commits[commitCount++] = commit;
            return true;
        }
        return false; // Unable to add commit
    }

    bool addFile(File* file) {
        if (fileCount < MAX_FILES) {
            files[fileCount++] = file;
            return true;
        }
        return false;
    }


    bool deleteFile(const string& fileName) {
        for (int i = 0; i < fileCount; i++) {
            if (files[i]->getName() == fileName) {
                delete files[i];
                files[i] = files[fileCount - 1];
                fileCount--;
                return true;
            }
        }
        return false; 
    }

   


    void setPublic(bool isPublic) {
        this->isPublic = isPublic;
    }


    const Commit** getCommits() const { return (const Commit**)commits; }

    const File** getFiles() const { return (const File**)files; }

    int getCommitCount() const { return commitCount; }

    int getFileCount() const { return fileCount; }

    bool isRepositoryPublic() const { return isPublic; }

    int getForkCount() const { return forkCount; }

    void incrementForkCount() { forkCount++; }

};

class Tree {
private:
    struct TreeNode {
        Repository* repository;
        TreeNode* left;
        TreeNode* right;

        TreeNode(Repository* repo) : repository(repo), left(nullptr), right(nullptr) {}
    };

    TreeNode* root;

    void deleteTree(TreeNode* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node->repository;
            delete node;
        }
    }

public:
    Tree() : root(nullptr) {}

    ~Tree() {
        deleteTree(root);
    }

    bool addRepository(Repository* repository) {
        TreeNode* newNode = new TreeNode(repository);
        if (!root) {
            root = newNode;
            return true;
        }

        TreeNode* current = root;
        TreeNode* parent = nullptr;

        while (current) {
            parent = current;
            if (repository->getName() < current->repository->getName()) {
                current = current->left;
            }
            else if (repository->getName() > current->repository->getName()) {
                current = current->right;
            }
            else {
                delete newNode;
                return false;
            }
        }

        if (repository->getName() < parent->repository->getName()) {
            parent->left = newNode;
        }
        else {
            parent->right = newNode;
        }

        return true;
    }

    Repository* searchRepository(const string& repoName) const {
        TreeNode* current = root;
        while (current) {
            if (repoName == current->repository->getName()) {
                return current->repository;
            }
            else if (repoName < current->repository->getName()) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }
        return nullptr;
    }
    bool deleteRepository(const string& repoName) {
        TreeNode* current = root;
        TreeNode* parent = nullptr;

        while (current) {
            if (repoName == current->repository->getName()) {
                if (parent) {
                    if (current == parent->left) {
                        parent->left = nullptr;
                    }
                    else {
                        parent->right = nullptr;
                    }
                }
                else {
                    root = nullptr;
                }

                delete current->repository;
                delete current;
                return true;
            }

            parent = current;
            if (repoName < current->repository->getName()) {
                current = current->left;
            }
            else {
                current = current->right;
            }
        }

        return false; 
    }

};

class User {
private:
    string username;
    string password;
    string followers[MAX_FOLLOWERS]; // Array to store followers
    int followerCount;
    unordered_map<string, Repository*> repositories; // Map to store user's repositories
    unordered_map<string, Repository*> forkedRepositories; // Map to store user's forked repositories

public:
    // Default constructor
    User() : username(""), password(""), followerCount(0) {}

    User(string uname, string pwd) : username(uname), password(pwd), followerCount(0) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    const string* getFollowers() const { return followers; }
    int getFollowerCount() const { return followerCount; }
    bool addFollower(const string& follower) {
        if (followerCount < MAX_FOLLOWERS) {
            followers[followerCount++] = follower;
            return true;
        }
        return false; 
    }

    void addRepository(Repository* repo) {
        repositories[repo->getName()] = repo;
    }


    void forkRepository(Repository* repo) {
        forkedRepositories[repo->getName()] = new Repository(repo->getName(), repo->isRepositoryPublic());
        repo->incrementForkCount();
        for (int i = 0; i < repo->getCommitCount(); i++) {
            forkedRepositories[repo->getName()]->addCommit(new Commit(repo->getCommits()[i]->getMessage()));
        }
        for (int i = 0; i < repo->getFileCount(); i++) {
            forkedRepositories[repo->getName()]->addFile(new File(repo->getFiles()[i]->getName()));
        }
    }



    bool deleteRepository(const string& repoName) {
        auto it = repositories.find(repoName);
        if (it != repositories.end()) {
            repositories.erase(it);
            return true;
        }
        return false; 
    }


    const unordered_map<string, Repository*>& getRepositories() const {
        return repositories;
    }
    const unordered_map<string, Repository*>& getForkedRepositories() const {
        return forkedRepositories;
    }
};



class UserRepository {
private:
    string name;
    string commits[MAX_FOLLOWERS]; 
    string files[MAX_FOLLOWERS];  
    int commitCount;
    int fileCount;

public:
    UserRepository(string repoName) : name(repoName), commitCount(0), fileCount(0) {}
    string getName() const { return name; }
    const string* getCommits() const { return commits; }
    const string* getFiles() const { return files; }
    int getCommitCount() const { return commitCount; }
    int getFileCount() const { return fileCount; }
    bool addCommit(const string& commit) {
        if (commitCount < MAX_FOLLOWERS) {
            commits[commitCount++] = commit;
            return true;
        }
        return false;
    }
    bool addFile(const string& file) {
        if (fileCount < MAX_FOLLOWERS) {
            files[fileCount++] = file;
            return true;
        }
        return false; 
    }
};

class SocialGraph {
private:
    string users[MAX_USERS];    
    int userCount;
    int followMatrix[MAX_USERS][MAX_USERS];

public:
    SocialGraph() : userCount(0) {
        for (int i = 0; i < MAX_USERS; ++i) {
            for (int j = 0; j < MAX_USERS; ++j) {
                followMatrix[i][j] = 0; 
            }
        }
    }

    bool addUser(const string& username) {
        if (userCount < MAX_USERS) {
            users[userCount++] = username;
            return true;
        }
        return false; 
    }

    bool followUser(int followerIndex, int followeeIndex) {
        if (followerIndex < userCount && followeeIndex < userCount) {
            followMatrix[followerIndex][followeeIndex] = 1;
            return true;
        }
        return false; 
    }

    bool unfollowUser(int followerIndex, int followeeIndex) {
        if (followerIndex < userCount && followeeIndex < userCount) {
            followMatrix[followerIndex][followeeIndex] = 0;
            return true;
        }
        return false; 
    }

    const string* getUsernames() const { return users; }
    int getUserCount() const { return userCount; }
    const int* getFollowMatrix() const { return *followMatrix; }
};

class UserManager {
private:

    unordered_map<string, User*> users;

    string dataFile = "data.xlsx"; 
    string datafile = "user_data.txt";

public:
    bool registerUser(const string& username, const string& password) {
        if (users.count(username) == 0) {
            users[username] = new User(username, password);
            return true;
        }
        return false; 
    }

    User* loginUser(const string& username, const string& password) {
        if (users.count(username) && users[username]->getPassword() == password) {
            return users[username];
        }
        return nullptr;
    }

    User* getUser(const string& username) {
        if (users.count(username)) {
            return users[username];
        }
        return nullptr; 
    }

    void saveUserData() {
        ofstream file(dataFile);
        if (file.is_open()) {
            for (auto it = users.begin(); it != users.end(); ++it) {
                file << it->first << "," << it->second->getPassword() << endl;
            }
            file.close();
            cout << "User data saved to file: " << dataFile << endl;
        }
        else {
            cout << "Unable to open file: " << dataFile << endl;
        }
    }


    void loadUserData() {
        ifstream file(dataFile);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                size_t pos = line.find(",");
                if (pos != string::npos) {
                    string username = line.substr(0, pos);
                    string password = line.substr(pos + 1);
                    users[username] = new User(username, password);
                }
            }
            file.close();
            cout << "User data loaded from file: " << dataFile << endl;
        }
        else {
            cout << "Unable to open file: " << dataFile << endl;
        }
    }

    ////////////////////////////////////////////////////////////////////


    void saveAllDataToFile() {
        ofstream file(datafile);
        if (file.is_open()) {
            for (const auto& pair : users) {
                User* user = pair.second;
                file << "Username: " << user->getUsername() << endl;
               
                const unordered_map<string, Repository*>& repositories = user->getRepositories();
                for (const auto& repoPair : repositories) {
                    Repository* repo = repoPair.second;
                    file << "Repository: " << repo->getName() << endl;

                    // Save commits
                    const Commit** commits = repo->getCommits();
                    for (int i = 0; i < repo->getCommitCount(); i++) {
                        file << "Commit: " << commits[i]->getMessage() << endl;
                    }

                    // Save files
                    const File** files = repo->getFiles();
                    for (int i = 0; i < repo->getFileCount(); i++) {
                        file << "File: " << files[i]->getName() << endl;
                    }
                }
            }
            file.close();
            cout << "All user data saved to file: " << datafile << endl;
        }
        else {
            cout << "Unable to open file for saving all user data." << endl;
        }
    }

    void loadAllDataFromFile() {
        ifstream file(datafile);
        if (file.is_open()) {
            string line;
            User* currentUser = nullptr;
            Repository* currentRepo = nullptr;

            while (getline(file, line)) {
                if (line.find("Username: ") == 0) {
                    string username = line.substr(10);
                    currentUser = users[username];
                }
                else if (line.find("Repository: ") == 0) {
                    string repoName = line.substr(12);
                    currentRepo = new Repository(repoName, false);
                    currentUser->addRepository(currentRepo);
                }
                else if (line.find("Commit: ") == 0 && currentRepo) {
                    string commitMessage = line.substr(8);
                    currentRepo->addCommit(new Commit(commitMessage));
                }
                else if (line.find("File: ") == 0 && currentRepo) {
                    string fileName = line.substr(6);
                    currentRepo->addFile(new File(fileName));
                }
            }

            file.close();
            cout << "All user data loaded from file: " << datafile << endl;
        }
        else {
            cout << "Unable to open file for loading all user data." << endl;
        }
    }


};

int main() {
    UserManager userManager;
    SocialGraph socialGraph;
    Tree repositoryTree;
    User* loggedInUser = nullptr;


    userManager.loadUserData(); 


    userManager.loadAllDataFromFile();



    while (true) 
    {

        cout << "\n===== GitHub-like Platform =====\n";
        cout << "1. Create Account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;
        if (choice == 1) 
        {
           

            // Create Account
            string username, password;
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;
            if (userManager.registerUser(username, password)) {
                cout << "Account created successfully!\n";
                socialGraph.addUser(username);
            }
            else {
                cout << "Unable to create account. Maximum users reached.\n";
            }
        }
        else if (choice == 2)
        {

            // Login
            string username, password;
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;
            loggedInUser = userManager.loginUser(username, password);
            if (loggedInUser) {
                cout << "Login successful! Welcome, " << username << "!\n";
                while (true) {
                    cout << "\n===== User Menu =====\n";
                    cout << "1. View Profile\n";
                    cout << "2. Follow User\n";
                    cout << "3. Unfollow User\n";
                    cout << "4. Create Repository\n";
                    cout << "5. Add Commit\n";
                    cout << "6. Add File\n";
                    cout << "7. View Repository Details\n";
                    cout << "8. Fork Repository\n";
                    cout << "9. Delete Repository\n";
                    cout << "10. Change Repository Visibility\n";
                    cout << "11. Delete File\n";
                    cout << "12. Return to Main Menu\n";
                    cout << "Enter your choice: ";

                    int userChoice;
                    cin >> userChoice;

                    if (userChoice == 1) {
                        // View profile
                        if (loggedInUser) {
                            cout << "Username: " << loggedInUser->getUsername() << endl;
                            cout << "Followers: " << loggedInUser->getFollowerCount() << endl;
                            cout << "Repositories: " << loggedInUser->getRepositories().size() << endl;
                            cout << "Forked Repositories: " << loggedInUser->getForkedRepositories().size() << endl;
                        }
                        else {
                            cout << "Please log in to view your profile.\n";
                        }
                    }
                    else if (userChoice == 2) {
                        // Follow user
                        if (loggedInUser) {
                            string followeeUsername;
                            cout << "Enter the username of the user you want to follow: ";
                            cin >> followeeUsername;
                            User* followee = userManager.getUser(followeeUsername);
                            if (followee) {
                                if (loggedInUser->addFollower(followeeUsername)) {
                                    cout << "You are now following " << followeeUsername << ".\n";
                                }
                                else {
                                    cout << "Unable to follow " << followeeUsername << ". Maximum followers reached.\n";
                                }
                            }
                            else {
                                cout << "User not found.\n";
                            }
                        }
                        else {
                            cout << "Please log in to follow a user.\n";
                        }
                    }
                    else if (userChoice == 3) {
                        // Unfollow user
                        if (loggedInUser) {
                            string unfolloweeUsername;
                            cout << "Enter the username of the user you want to unfollow: ";
                            cin >> unfolloweeUsername;

                            // Find the index of the unfollowee in the social graph
                            int unfolloweeIndex = -1;
                            const string* usernames = socialGraph.getUsernames();
                            for (int i = 0; i < socialGraph.getUserCount(); i++) {
                                if (usernames[i] == unfolloweeUsername) {
                                    unfolloweeIndex = i;
                                    break;
                                }
                            }

                            int followerIndex = -1;
                            for (int i = 0; i < socialGraph.getUserCount(); i++) {
                                if (usernames[i] == loggedInUser->getUsername()) {
                                    followerIndex = i;
                                    break;
                                }
                            }

                            if (followerIndex != -1 && unfolloweeIndex != -1) {
                                if (socialGraph.unfollowUser(followerIndex, unfolloweeIndex)) {
                                    cout << "You have unfollowed " << unfolloweeUsername << ".\n";
                                }
                                else {
                                    cout << "Failed to unfollow " << unfolloweeUsername << ". Invalid follower or followee index.\n";
                                }
                            }
                            else {
                                cout << "User not found.\n";
                            }
                        }
                        else {
                            cout << "Please log in to unfollow a user.\n";
                        }
                    }
                    else if (userChoice == 4) {
                        system("cls");

                        // Create Repository
                        string repoName;
                        bool isPublic;
                        cout << "Enter repository name: ";
                        cin >> repoName;
                        cout << "Make the repository public? (1 for yes, 0 for no): ";
                        cin >> isPublic;
                        Repository* newRepo = new Repository(repoName, isPublic);
                        if (repositoryTree.addRepository(newRepo)) {
                            cout << "Repository created successfully!\n";
                        }
                        else {
                            cout << "Repository with the same name already exists.\n";
                            delete newRepo; 
                        }
                    }
                    else if (userChoice == 5) {
                        system("cls");


                        // Add Commit
                        string repoName, commitMessage;
                        cout << "Enter repository name: ";
                        cin >> repoName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            cout << "Enter commit message: ";
                            cin.ignore(); 
                            getline(cin, commitMessage);
                            Commit* newCommit = new Commit(commitMessage);
                            if (repo->addCommit(newCommit)) {
                                cout << "Commit added successfully!\n";
                            }
                            else {
                                cout << "Failed to add commit. Maximum commits reached.\n";
                                delete newCommit; 
                            }
                        }
                        else {
                            cout << "Repository not found.\n";
                        }
                    }
                    else if (userChoice == 6) {
                        system("cls");

                        // Add File
                        string repoName, fileName;
                        cout << "Enter repository name: ";
                        cin >> repoName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            cout << "Enter file name: ";
                            cin >> fileName;
                            File* newFile = new File(fileName);
                            if (repo->addFile(newFile)) {
                                cout << "File added successfully!\n";
                            }
                            else {
                                cout << "Failed to add file. Maximum files reached.\n";
                                delete newFile; 
                            }
                        }
                        else {
                            cout << "Repository not found.\n";
                        }
                    }
                    else if (userChoice == 7) {
                        // View Repository Details
                        string repoName;
                        cout << "Enter repository name: ";
                        cin >> repoName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            cout << "Repository Name: " << repo->getName() << endl;
                            cout << "Repo Visibility: " << (repo->isRepositoryPublic() ? "Public" : "Private") << endl;
                            cout << "Repo Fork Count: " << repo->getForkCount() << endl;
                            cout << "Commits:" << endl;
                            const Commit** commits = repo->getCommits();
                            for (int i = 0; i < repo->getCommitCount(); ++i) {
                                cout << "- " << commits[i]->getMessage() << endl;
                            }
                            cout << "Files:" << endl;
                            const File** files = repo->getFiles();
                            for (int i = 0; i < repo->getFileCount(); ++i) {
                                cout << "- " << files[i]->getName() << endl;
                            }
                        }
                        else {
                            cout << "Repository not found.\n";
                        }
                    }



                    else if (userChoice == 8) {
                        system("cls");

                        // Fork Repository
                        string repoName;
                        cout << "Enter the name of the repository you want to fork: ";
                        cin >> repoName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            if (loggedInUser) {
                                loggedInUser->forkRepository(repo);
                                cout << "Repository forked successfully!\n";
                            }
                            else {
                                cout << "Please log in to fork a repository.\n";
                            }
                        }
                        else {
                            cout << "Repository not found.\n";
                        }
                    }
                    else if (userChoice == 9) {
                        system("cls");

                        // Delete Repository
                        string repoName;
                        cout << "Enter the name of the repository you want to delete: ";
                        cin >> repoName;
                        if (loggedInUser) {
                            if (loggedInUser->deleteRepository(repoName)) {
                                if (repositoryTree.deleteRepository(repoName)) {
                                    cout << "Repository deleted successfully!\n";
                                }
                                else {
                                    // This should not happen, as the repository was just deleted from the user's list
                                    cout << "Error deleting repository from the tree.\n";
                                }
                            }
                            else {
                                cout << "Repository not found in your repositories.\n";
                            }
                        }
                        else {
                            cout << "Please log in to delete a repository.\n";
                        }
                    }
                    else if (userChoice == 10) {
                        // Change Repository Visibility
                        string repoName;
                        cout << "Enter the name of the repository you want to change the visibility: ";
                        cin >> repoName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            bool newVisibility;
                            cout << "Enter the new visibility (1 for public, 0 for private): ";
                            cin >> newVisibility;
                            repo->setPublic(newVisibility);
                            cout << "Repository visibility updated successfully.\n";
                        }
                        else {
                            cout << "Repository not found.\n";
                        }
                    }

                   


                    else if (userChoice == 11) {
                        // Delete File
                        string repoName, fileName;
                        cout << "Enter the name of the repository: ";
                        cin >> repoName;
                        cout << "Enter the name of the file you want to delete: ";
                        cin >> fileName;
                        Repository* repo = repositoryTree.searchRepository(repoName);
                        if (repo) {
                            if (repo->deleteFile(fileName)) {
                                cout << "File '" << fileName << "' deleted successfully from repository '" << repoName << "'.\n";
                                if (!repositoryTree.deleteRepository(repoName) || !repositoryTree.addRepository(repo)) {
                                    cout << "Error updating the repository in the tree.\n";
                                }
                            }
                            else {
                                cout << "File '" << fileName << "' not found in repository '" << repoName << "'.\n";
                            }
                        }
                        else {
                            cout << "Repository '" << repoName << "' not found.\n";
                        }
                        }



                    else if (userChoice == 12) {
                        cout << "Returning  to Main Menu" << endl;
                        system("cls");


                        break; 
                    }



                    else {
                        cout << "Invalid choice. Please enter a valid option.\n";
                    }
                }
            }
            else {
                cout << "Invalid username or password. Please try again.\n";
            }
        }
        else if (choice == 3)
        {
            userManager.saveUserData(); 


            userManager.saveAllDataToFile();

            cout << "Exiting program...\n";
            return 0;
        }
        else {
            cout << "Invalid choice. Please enter a valid option.\n";
        }
    }

    return 0;
}
