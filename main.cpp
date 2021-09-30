#include<bits/stdc++.h>
#include <unordered_map>
#include <unordered_set>
using namespace std;


struct edge {
    string src, dest;
};

struct URLkeywords {
    string url;
    vector<string> keywords;
};

struct URLimpression {
    string url;
    int impression;
};

class webEngine
{
private:
    unordered_map<string, int> urlIdx;
    vector<string> idxURL;
    vector<vector<int>> adjList;
    unordered_map<string, vector<int>> keywords, case_sensitive_keywords;
    vector<int> outDegree, impressions, clicks;
    vector<double> PR, CTR;
    vector<string> search_result;
    int numberOfURLs;
    int state = 0;

    void mapToIndecies(vector<edge> const& edges) {
        int idx = 0;
        for (const auto& i : edges)
        {
            if (urlIdx.find(i.src) == urlIdx.end()) {
                urlIdx[i.src] = idx++;
                idxURL.push_back(i.src);
            }
            if (urlIdx.find(i.dest) == urlIdx.end()) {
                urlIdx[i.dest] = idx++;
                idxURL.push_back(i.dest);
            }
        }
        numberOfURLs = idx;


        //Resizing the datastructures with the number of urls and fill with relevant values
        adjList.resize(numberOfURLs);
        outDegree.resize(numberOfURLs, 0);
        impressions.resize(numberOfURLs, 0);
        clicks.resize(numberOfURLs, 0);
        CTR.resize(numberOfURLs, 0);
        PR.resize(numberOfURLs, 1 / (double)numberOfURLs);
    }

    void calculatePageRank() {
        int iterations = 100, n = numberOfURLs;

        double damping = 0.85;
        vector<double> current(numberOfURLs, 0);

        while (iterations--) {
            for (int url = 0; url < numberOfURLs; url++) {
                current[url] = (1 - damping);
                for (const auto& link : adjList[url])
                    current[url] += damping * (PR[link] / (double)outDegree[link]);
            }
            PR = current;
            current.resize(numberOfURLs, 0);
        }
    }

    void updateClicks(string surl) {
        int url = urlIdx[surl];
        clicks[url]++;
        CTR[url] = (double)clicks[url] / (double)impressions[url];
    }

    void updateImpression(int url) {
        impressions[url]++;
        CTR[url] = (double)clicks[url] / (double)impressions[url];
    }

    double calculatScore(int url) {
        double fraction = (0.1 * impressions[url]) / (1 + 0.1 * impressions[url]);
        return 0.4 * PR[url] + ((1.0 - fraction) * PR[url] + fraction * CTR[url]) * 0.6;
    }

    void view_website() {
        state = 2;
        int choice;
        cout << "Enter the number of the web page:  ";
        cin >> choice;
        cin.ignore();
        if (search_result.size() >= choice) {
            updateClicks(search_result[choice - 1]);
            cout << "\nYou are now viewing " << search_result[choice - 1] << endl;
        }
        else
            cout << "Invalid website number\n";
        cout << "\n\nWhat would you like to do ? \n1.  Return to search result\n2.  New Search\n3.  Exit\n\nType in your choice : ";

    }

    void print_result() {
        state = 1;
        int no = 1;
        cout << endl;
        for (auto& i : search_result)
        {
            cout << no++ << ".  " << i << endl;
        }
        cout << "\n\nWhat would you like to do ? \n1.  Choose a web page to open\n2.  New Search\n3.  Exit\n\nType in your choice : ";
    }

    void search() {
        cout << "Enter your Search Query:  ";
        string query;
        getline(cin, query);

        search_result.clear();

        priority_queue<pair<double, int>> scores;
        vector<int> indicies;
        vector<string> words;
        int type = 3;

        //Find type of query
        if (query[0] == '"') {
            words.push_back(query.substr(1, query.size() - 2));
            type = 1;
        }
        else {
            stringstream ss(query);
            string temp;
            while (ss >> temp) {
                if (temp == "AND") {
                    type = 2;
                    continue;
                }
                else if (temp == "OR") continue;
                words.push_back(temp);
            }
        }

        vector<int> vis(numberOfURLs, 0);
        if (type == 2) {
            for (const auto& word : words) {
                string low_word;
                for (auto& c : word)
                    low_word += tolower(c);
                for (auto const& i : keywords[low_word]) {
                    vis[i]++;
                    if (vis[i] == words.size())
                        scores.push({ calculatScore(i), i });
                }
            }
        }
        else if (type == 1) {
            for (auto const& i : case_sensitive_keywords[words[0]])
                scores.push({ calculatScore(i), i });
        }
        else {
            for (const auto& word : words) {
                string low_word;
                for (auto& c : word)
                    low_word += tolower(c);
                for (auto const& i : keywords[low_word]) {
                    if (!vis[i]) {
                        scores.push({ calculatScore(i), i });
                        vis[i]++;
                    }
                }
            }
        }

        while (scores.size()) {
            updateImpression(scores.top().second);
            search_result.push_back(idxURL[scores.top().second]);
            scores.pop();
        }

        print_result();
    }

    void saveNewData() {
        fstream f;

        //update impressions
        f.open("impressions.csv", fstream::out);
        for (int i = 0; i < numberOfURLs; i++) {
            f << idxURL[i] << ", " << impressions[i];
            if (i < numberOfURLs - 1) f << endl;
        }
        f.close();

        //save clicks
        f.open("clicks.csv", fstream::out);
        for (int i = 0; i < numberOfURLs; i++) {
            f << idxURL[i] << ", " << clicks[i];
            if (i < numberOfURLs - 1) f << endl;
        }
        f.close();
    }

public:


    webEngine(vector<edge> const& edges, vector<URLkeywords> const& inKeywords, vector<URLimpression> const& inImpressions)
    {
        //Mapping the URLs to integers to give an idex for each url, counting their number and resizing all the datastructures used
        mapToIndecies(edges);



        //Creating the Adjecency List transposed to have the websites pointing to each website and calculating the outdegree of each website
        for (const auto& i : edges) {
            int a = urlIdx[i.src], b = urlIdx[i.dest];
            adjList[b].push_back(a);
            outDegree[a]++;
        }


        //Creating the keywords List
        for (auto const& i : inKeywords) {
            int url = urlIdx[i.url];
            for (auto const& key : i.keywords) {
                string low_temp;
                for (auto& c : key)
                    low_temp += tolower(c);
                keywords[low_temp].push_back(url);
                case_sensitive_keywords[key].push_back(url);
            }
        }

        //Assigning the impressions to each URL
        for (auto const& i : inImpressions) {
            int url = urlIdx[i.url];
            impressions[url] = i.impression;
        }

        //Calculate Page Rank
        calculatePageRank();
    }



    bool interact(int option) {
        if (option == 3) {
            saveNewData();
            return 0;
        }

        if (option == 2) {
            search();
            return true;
        }

        else if (option == 1 && state == 1) {
            view_website();
            return true;
        }
        else if (option == 1 && state == 2) {
            print_result();
            return true;
        }
        else {
            cout << "Inavlid input!\n";
            return true;
        }
    }

};


vector<vector<string>> processCSV(string name) {
    fstream f;
    f.open(name);
    if (f.fail())
    {
        cout << "Could not process " << name << "!\n";
        exit(1);
    }

    vector<vector<string>> output;
    vector<string> row;
    string line, word;

    while (!f.eof()) {
        row.clear();

        getline(f, line);
        stringstream ss(line);

        while (getline(ss, word, ','))
            row.push_back(word);

        output.push_back(row);
    }
    f.close();
    return output;
}


int main() {


    //Processing the edges between URLs
    vector<vector<string>> temp = processCSV("urls.csv");
    vector<edge> edges;

    for (auto& i : temp) {
        edges.push_back({ i[0],i[1] });
    }

    //Processing the keywords of each URL
    temp = processCSV("keywords.csv");
    vector<URLkeywords> urlKeywords;

    for (auto& i : temp) {
        string url = i[0];
        vector<string> keys;
        for (int j = 1; j < i.size(); j++)
            keys.push_back(i[j]);

        urlKeywords.push_back({ url, keys });
        i.clear();
    }

    //Processing the initial impression of each URL
    temp = processCSV("impressions.csv");
    vector<URLimpression> urlImpressions;

    for (auto& i : temp) {
        string url = i[0];
        int imp = stoi(i[1]);
        urlImpressions.push_back({ url, imp });
    }

    //Creating an object of type webEngine
    webEngine myWeb(edges, urlKeywords, urlImpressions);

    //Starting the program
    int option, choice;
    string query;
    bool exit = 0;
    vector<string> result;
    cout << "Welcome!\nWhat would you like to do?\n1.  New Search\n2.  Exit\n\nType in your choice: ";
    cin >> option;
    if (option == 2) option = 3;
    else option = 2;
    cin.ignore();

    while (myWeb.interact(option)) {
        cin >> option;
        cin.ignore();
    }



}