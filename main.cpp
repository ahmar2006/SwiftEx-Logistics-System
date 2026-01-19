#include <iostream>
#include <string>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#define CLEAR_CMD "cls"
#else
#define CLEAR_CMD "clear"
#endif

using namespace std;

const int STATUS_PICKUP_QUEUE = 0;
const int STATUS_WAREHOUSE = 1;
const int STATUS_LOADING = 2;
const int STATUS_IN_TRANSIT = 3;
const int STATUS_DELIVERY_ATTEMPT = 4;
const int STATUS_DELIVERED = 5;
const int STATUS_RETURNED = 6;
const int STATUS_MISSING = 7;
const int STATUS_CANCELLED = 8;

void clearScreen() 
{
    system(CLEAR_CMD);
}

void pauseFunc() 
{
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

string getCurrentTimestamp() 
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << setfill('0') << setw(2) << ltm->tm_hour << ":" 
       << setw(2) << ltm->tm_min << ":" << setw(2) << ltm->tm_sec;
    return ss.str();
}

class IntArrayList 
{
private:
    int* data; 
    int capacity; 
    int count;
    
    void resize(int newCapacity) 
    {
        int* newData = new int[newCapacity];
        for (int i = 0; i < count; i++) 
            newData[i] = data[i];
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    IntArrayList() : capacity(50), count(0) { data = new int[capacity]; } 
    
    IntArrayList(const IntArrayList& other) 
    {
        capacity = other.capacity; count = other.count; data = new int[capacity];
        for(int i=0; i<count; i++) data[i] = other.data[i];
    }
    
    IntArrayList& operator=(const IntArrayList& other) 
    {
        if (this != &other) 
        {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = new int[capacity];
            for(int i=0; i<count; i++) 
                data[i] = other.data[i];
        }
        return *this;
    }

    ~IntArrayList() 
    {
        delete[] data; 
    }
    
    void add(int val) 
    { 
        if (count == capacity) 
            resize(capacity * 2); 
        data[count++] = val; 
    }
    
    int get(int index) const 
    { 
        return (index >= 0 && index < count) ? data[index] : -1;
    }
    
    int size() const 
    {
        return count; 
    }
    
    bool isEmpty() const 
    { 
        return count == 0; 
    }
};

class MapGraph;

struct HistoryEvent 
{
    string description;
    string time;
    string location;
    HistoryEvent* next;
    HistoryEvent(string d, string t, string l) : description(d), time(t), location(l), next(nullptr) {}
};

class TrackingHistory 
{
private:
    HistoryEvent* head;
    HistoryEvent* tail;

public:
    TrackingHistory() : head(nullptr), tail(nullptr) {}
    
    void addEvent(string desc, string loc) 
    {
        HistoryEvent* newEvent = new HistoryEvent(desc, getCurrentTimestamp(), loc);
        if (!head) 
        {
            head = tail = newEvent;
        }
        else 
        {
            tail->next = newEvent;
            tail = newEvent;
        }
    }

    void printTimeline() 
    {
        cout << "\n--- [TRACKING HISTORY] ---\n";
        HistoryEvent* curr = head;
        while (curr) 
        {
            cout << " [" << curr->time << "] " << curr->description << " @ " << curr->location << endl;
            curr = curr->next;
        }
        cout << "--------------------------\n";
    }
    
    TrackingHistory(const TrackingHistory& other) : head(nullptr), tail(nullptr) 
    {
        HistoryEvent* curr = other.head;
        
        while (curr) 
        {
            addEvent(curr->description, curr->location);
            curr = curr->next;
        }
    }
};

struct Parcel 
{
    string id;
    string destination;
    double weight;
    int priority; 
    int status;
    int priorityScore;
    string assignedRider;
    
    string weightCategory; 
    string zone;           
    int deliveryAttempts;  
    
    long long dispatchTime; 
    long long lastUpdateTime; 
    long long arrivalTime;
    long long lastKnownTime;
    
    TrackingHistory* history; 
    
    IntArrayList* currentRoute;
    int currentRouteDistance;
    int currentPosition;
    
    Parcel() : weight(0), priority(1), status(0), priorityScore(0), 
               history(new TrackingHistory()), dispatchTime(0), lastUpdateTime(0), 
               arrivalTime(0), deliveryAttempts(0), currentRoute(nullptr),
               currentRouteDistance(0), currentPosition(0), lastKnownTime(0) {}
    
    Parcel(string pid, string dest, double w, int p, string z) : 
           id(pid), destination(dest), weight(w), priority(p), 
           status(STATUS_PICKUP_QUEUE), lastUpdateTime(0), arrivalTime(0), 
           zone(z), deliveryAttempts(0), dispatchTime(0), currentRoute(nullptr),
           currentRouteDistance(0), currentPosition(0), lastKnownTime(0) 
    {
        priorityScore = p * 1000 + (int)w; 
        
        if (w < 5.0) 
            weightCategory = "Light";
        else if (w < 20.0) 
            weightCategory = "Medium";
        else 
            weightCategory = "Heavy";

        history = new TrackingHistory();
        history->addEvent("Pickup Request Created", "Customer Loc");
    }
    
    void updateStatus(int newStatus, string desc, string loc) 
    {
        status = newStatus;
        history->addEvent(desc, loc);
        lastUpdateTime = time(0);
        lastKnownTime = time(0);
    }

    string getStatusString() const 
    {
        switch (status) 
        {
            case 0: return "Pickup Queue";
            case 1: return "Warehouse (Sorted)";
            case 2: return "Loading Vehicle";
            case 3: return "In Transit";
            case 4: return "Out for Delivery";
            case 5: return "Delivered";
            case 6: return "Returned to Sender";
            case 7: return "MISSING";
            case 8: return "Cancelled";
            default: return "Unknown";
        }
    }
    
    ~Parcel() {
        if (currentRoute) delete currentRoute;
    }
};

ostream& operator<<(ostream& os, const Parcel& p) 
{
    os << left << setw(6) << p.id 
       << setw(12) << p.destination 
       << setw(8) << p.weightCategory
       << setw(8) << p.zone
       << " [" << p.getStatusString() << "]";
    return os;
}

struct Rider {
    string name;
    string type;
    int currentLoad;
    int maxLoad;
    bool available;
    int parcelsAssigned;
    
    Rider(string n, string t, int max) : 
          name(n), type(t), currentLoad(0), maxLoad(max), 
          available(true), parcelsAssigned(0) {}
};

class StringQueue 
{
private:
    struct Node 
    { 
        string data; 
        Node* next; 
        
        Node(string d):data(d),next(nullptr){} 
    };
    
    Node* front; 
    Node* rear;

public:
    StringQueue() : front(nullptr), rear(nullptr) {}
    
    void enqueue(string val) 
    {
        Node* n = new Node(val);
        if(!rear) 
            front = rear = n; 
        else 
        {
            rear->next = n;
            rear = n; 
        }
    }
    
    string dequeue() 
    {
        if(!front) 
            return "";
        
        Node* temp = front; 
        string val = front->data; 
        front = front->next;
        
        if(!front) 
            rear = nullptr; 
        delete temp; 
        return val;
    }
    
    bool isEmpty() 
    { 
        return front == nullptr; 
    }
};

class ParcelArrayList 
{
private:
    Parcel** data; 
    int capacity; 
    int count;

    void resize(int newCapacity) 
    {
        Parcel** newData = new Parcel*[newCapacity];
        for (int i = 0; i < count; i++) 
            newData[i] = data[i];
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    ParcelArrayList() : capacity(20), count(0) 
    { 
        data = new Parcel*[capacity]; 
    }
    
    ~ParcelArrayList() 
    {
        delete[] data; 
    }
    
    void add(Parcel* val) 
    { 
        if (count == capacity) 
            resize(capacity * 2); 
        data[count++] = val; 
    }
    
    void removeLast() 
    {
        if (count > 0) count--; 
    }
    
    Parcel* get(int index) const 
    { 
        return (index >= 0 && index < count) ? data[index] : nullptr; 
    }
    
    void set(int index, Parcel* val) 
    {
        if (index >= 0 && index < count) 
            data[index] = val; 
    }
    
    int size() const 
    {
        return count; 
    }
    
    bool isEmpty() const 
    { 
        return count == 0; 
    }
    
    void swap(int i, int j) 
    { 
        Parcel* temp = data[i]; 
        data[i] = data[j]; 
        data[j] = temp; 
    }
};

struct ParcelNode 
{
    Parcel* data; 
    ParcelNode* next;
    
    ParcelNode(Parcel* val) : data(val), next(nullptr) {}
};

struct Edge 
{ 
    int dest; 
    int weight; 
    bool blocked; 
    int currentLoad;
    int maxLoad;
    
    Edge(int d=0, int w=0, int maxL=10) : 
        dest(d), weight(w), blocked(false), currentLoad(0), maxLoad(maxL) {} 
    
    bool isOverloaded() {
        return currentLoad >= maxLoad;
    }
};

class EdgeArrayList 
{
private:
    Edge* data; 
    int capacity; 
    int count;
    
    void resize(int newCapacity) 
    {
        Edge* newData = new Edge[newCapacity]; 
        for(int i=0; i<count; i++) 
            newData[i] = data[i];
        
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    EdgeArrayList() : capacity(5), count(0) 
    { 
        data = new Edge[capacity]; 
    }
    
    EdgeArrayList(const EdgeArrayList& other) 
    {
        capacity = other.capacity; 
        count = other.count; 
        data = new Edge[capacity];
        
        for(int i=0; i<count; i++) 
            data[i] = other.data[i];
    }
    
    EdgeArrayList& operator=(const EdgeArrayList& other) 
    {
        if(this != &other) 
        {
            delete[] data; 
            capacity = other.capacity; 
            count = other.count; 
            data = new Edge[capacity];
            
            for(int i=0; i<count; i++) 
                data[i] = other.data[i];
        }
        return *this;
    }

    ~EdgeArrayList() 
    { 
        delete[] data; 
    }
    
    void add(Edge val) 
    { 
        if (count == capacity) 
            resize(capacity * 2); 
        data[count++] = val; 
    }
    
    int size() const 
    { 
        return count; 
    }
    
    Edge& getRef(int i) 
    { 
        return data[i]; 
    }
};

struct CityNode 
{ 
    string name;
    string zone; 
    EdgeArrayList edges; 
    
    CityNode(string n="", string z="") : name(n), zone(z) {} 
};

class MapGraph 
{
public:
    CityNode* cities; 
    int cityCount; 
    int cityCapacity;
    
    IntArrayList availablePaths[5]; 
    int availablePathDistances[5];
    int pathCount;

    MapGraph() : cityCount(0), cityCapacity(200) 
    { 
        cities = new CityNode[cityCapacity]; 
    }

    int addCity(string name, string zone) 
    { 
        if (cityCount == cityCapacity) 
            return -1; 
        cities[cityCount] = CityNode(name, zone); 
        return cityCount++; 
    }
    
    void addRoad(int u, int v, int dist, int maxLoad = 10) 
    { 
        cities[u].edges.add(Edge(v, dist, maxLoad)); 
        cities[v].edges.add(Edge(u, dist, maxLoad)); 
    }
    
    int getCityIndex(string name) 
    { 
        for(int i=0; i<cityCount; i++) 
            if (cities[i].name == name) 
                return i; 
        
        return -1; 
    }
    
    string getZone(string name) 
    {
        int idx = getCityIndex(name);
        if(idx != -1) 
            return cities[idx].zone;

        return "Unknown";
    }

    void displayAllCities() 
    {
        cout << "\n=========================================\n      AVAILABLE CITIES FOR DELIVERY\n=========================================\n";
        cout << left << setw(25) << "CITY NAME" << setw(15) << "ZONE" << endl;
        cout << "-----------------------------------------\n";
        
        for(int i=0; i<cityCount; i++) 
        {
            cout << left << setw(25) << cities[i].name << setw(15) << cities[i].zone << endl;
        }
        
        cout << "\nTotal Cities: " << cityCount << endl;
        cout << "=========================================\n";
    }

    void blockRandomRoad() 
    {
        if (cityCount < 2) 
            return;
        int u = rand() % cityCount;
        if (cities[u].edges.size() > 0) 
        {
            int eIdx = rand() % cities[u].edges.size();
            cities[u].edges.getRef(eIdx).blocked = true;
            cout << ">>> [ALERT] Road near " << cities[u].name << " is now BLOCKED!\n";
        }
    }
    
    void checkOverloads() {
        for(int i=0; i<cityCount; i++) {
            EdgeArrayList& edges = cities[i].edges;
            for(int k=0; k<edges.size(); k++) {
                Edge& e = edges.getRef(k);
                if (e.isOverloaded() && !e.blocked) {
                    e.blocked = true;
                    cout << ">>> [ALERT] Road " << cities[i].name << " -> " 
                         << cities[e.dest].name << " is OVERLOADED! Temporarily blocked.\n";
                }
            }
        }
    }
    
    void updateEdgeLoad(IntArrayList& path, int increment) {
        for(int i=0; i<path.size()-1; i++) {
            int city1 = path.get(i);
            int city2 = path.get(i+1);
            
            EdgeArrayList& edges = cities[city1].edges;
            for(int j=0; j<edges.size(); j++) {
                if (edges.getRef(j).dest == city2) {
                    edges.getRef(j).currentLoad += increment;
                    break;
                }
            }
        }
    }

    void displayNetwork() 
    {
        clearScreen();
        cout << "\n=========================================\n      LOGISTICS NETWORK MAP VIEW\n=========================================\n";
        for(int i=0; i<cityCount; i++) 
        {
            cout << " [" << cities[i].zone << "] " << left << setw(12) << cities[i].name << " connects to:\n";
            EdgeArrayList& edges = cities[i].edges;
            
            for(int k=0; k<edges.size(); k++) 
            {
                Edge& e = edges.getRef(k);
                cout << "    --> " << setw(12) << cities[e.dest].name << " | " << e.weight << "km";
                if(e.blocked) cout << " [BLOCKED]";
                if(e.isOverloaded()) cout << " [OVERLOADED: " << e.currentLoad << "/" << e.maxLoad << "]";
                else if(e.currentLoad > 0) cout << " [Load: " << e.currentLoad << "/" << e.maxLoad << "]";
                cout << endl;
            }
            cout << "-----------------------------------------\n";
        }
    }

    void findAllPaths(int start, int end) 
    {
        bool* visited = new bool[cityCount];
        for(int i=0; i<cityCount; i++) 
            visited[i] = false;
        pathCount = 0;
        IntArrayList currentPath;
        
        solveDFS(start, end, visited, currentPath, 0);
        
        delete[] visited;
    }

    void solveDFS(int u, int d, bool visited[], IntArrayList currentPath, int currentDist) 
    {
        visited[u] = true;
        currentPath.add(u);
        
        if (u == d) 
        {
             if (pathCount < 5) 
             {
                availablePaths[pathCount] = currentPath; 
                availablePathDistances[pathCount] = currentDist;
                pathCount++;
            }
        } 
        else 
        {
            EdgeArrayList& edges = cities[u].edges;
            for(int i=0; i<edges.size(); i++) 
            {
                Edge& e = edges.getRef(i);
                if(!visited[e.dest] && !e.blocked && !e.isOverloaded()) 
                {
                    solveDFS(e.dest, d, visited, currentPath, currentDist + e.weight);
                }
            }
        }
        visited[u] = false;
    }
    
    int getMinRouteIndex() 
    {
        int minIdx = -1;
        int minDist = INT_MAX;
        for(int i=0; i<pathCount; i++) 
        {
            if(availablePathDistances[i] < minDist) 
            {
                minDist = availablePathDistances[i];
                minIdx = i;
            }
        }
        return minIdx;
    }
};

class ParcelLinkedList 
{
private:
    ParcelNode* head; 
    ParcelNode* tail;
    
public:
    ParcelLinkedList() : head(nullptr), tail(nullptr) {}
    
    void pushBack(Parcel* val) 
    {
        ParcelNode* newNode = new ParcelNode(val);
        if (!tail) 
        { 
            head = tail = newNode; 
        } 
        else 
        { 
            tail->next = newNode; 
            tail = newNode; 
        }
    }
    
    void recalculateRoutes(MapGraph* map) {
        ParcelNode* curr = head;
        while(curr) 
        {
            Parcel* p = curr->data;
            if (p->status == STATUS_IN_TRANSIT && p->currentRoute) {
                bool routeBlocked = false;
                for(int i = p->currentPosition; i < p->currentRoute->size() - 1; i++) {
                    int city1 = p->currentRoute->get(i);
                    int city2 = p->currentRoute->get(i + 1);
                    if (rand() % 20 == 0) {
                        routeBlocked = true;
                        break;
                    }
                }
                
                if (routeBlocked) {
                    int start = p->currentRoute->get(p->currentPosition);
                    int end = map->getCityIndex(p->destination);
                    map->findAllPaths(start, end);
                    
                    if (map->pathCount > 0) {
                        int minIdx = map->getMinRouteIndex();
                        delete p->currentRoute;
                        p->currentRoute = new IntArrayList(map->availablePaths[minIdx]);
                        p->currentRouteDistance = map->availablePathDistances[minIdx];
                        p->history->addEvent("Route Recalculated Due to Blockage", "System");
                        cout << ">>> Route recalculated for parcel " << p->id << endl;
                    }
                }
            }
            curr = curr->next;
        }
    }
    
    void updateLifecycle(long long currentTime, MapGraph* map) 
    {
        ParcelNode* curr = head;
        while(curr) 
        {
            Parcel* p = curr->data;
            
            if (p->status == STATUS_IN_TRANSIT && (currentTime - p->lastKnownTime) > 15) {
                p->updateStatus(STATUS_MISSING, "No Signal for 15+ Seconds - Marked Missing", "Unknown");
                cout << ">>> ALERT: Parcel " << p->id << " marked as MISSING!\n";
            }
            
            if (p->status == STATUS_LOADING) 
            {
                if (currentTime >= p->lastUpdateTime + 5) 
                {
                    p->updateStatus(STATUS_IN_TRANSIT, "Vehicle Departed", "On Road");
                    p->lastKnownTime = currentTime;
                }
            }
            else if (p->status == STATUS_IN_TRANSIT) 
            {
                if (p->currentRoute && p->currentPosition < p->currentRoute->size() - 1) {
                    if (currentTime % 3 == 0) {
                        p->currentPosition = min(p->currentPosition + 1, p->currentRoute->size() - 1);
                    }
                }
                
                if (rand() % 1000 == 0) 
                {
                    p->updateStatus(STATUS_MISSING, "Signal Lost - Investigation Started", "Unknown");
                }
                else if (currentTime >= p->arrivalTime) 
                {
                    p->updateStatus(STATUS_DELIVERY_ATTEMPT, "Arrived at Destination Hub", p->destination);
                }
            }
            else if (p->status == STATUS_DELIVERY_ATTEMPT) 
            {
                if (rand() % 10 < 8) 
                {
                    p->updateStatus(STATUS_DELIVERED, "Handed to Recipient", "Doorstep");
                } 
                else 
                {
                    p->deliveryAttempts++;
                    if (p->deliveryAttempts >= 3) 
                    {
                        p->updateStatus(STATUS_RETURNED, "Max Attempts Reached - RTS", "Local Hub");
                    } 
                    else 
                    {
                        p->updateStatus(STATUS_IN_TRANSIT, "Recipient Unavailable - Retrying", "Local Hub");
                        p->arrivalTime = currentTime + 5; 
                    }
                }
            }
            curr = curr->next;
        }
    }

    void showTransitStatus(long long currentTime) 
    {
        ParcelNode* curr = head; bool headerPrinted = false;
        while(curr) 
        {
            if (curr->data->status == STATUS_IN_TRANSIT || curr->data->status == STATUS_LOADING) 
            {
                if (!headerPrinted) 
                { 
                    cout << "\n--- [LIVE TRANSIT MONITOR] ---\n"; 
                    headerPrinted = true; 
                }
                
                long long total = curr->data->arrivalTime - curr->data->dispatchTime;
                long long elapsed = currentTime - curr->data->dispatchTime;
                
                string state = (curr->data->status == STATUS_LOADING) ? "[LOADING] " : "[MOVING]  ";
                
                if(total<=0) total=1;
                double pct = (double)elapsed/total;
                if(pct>1.0) pct=1.0;
                
                cout << state << curr->data->id << " -> " << curr->data->destination << ": [";
                int bars = (int)(pct * 20);
                for(int i=0; i<20; i++) cout << (i < bars ? "=" : " ");
                cout << "] " << (int)(pct * 100) << "%";
                
                if (curr->data->currentRoute && curr->data->currentPosition > 0) {
                    cout << " (At position " << curr->data->currentPosition << "/" 
                         << curr->data->currentRoute->size() << ")";
                }
                cout << "\n";
            }
            curr = curr->next;
        }
        
        if (!headerPrinted) 
            cout << "\n(No parcels active in transit)\n";
    }
};

class ParcelHeap 
{
private:
    ParcelArrayList heap;
    
    void heapifyUp(int index) 
    {
        while (index > 0) 
        {
            int parent = (index - 1) / 2;
            if (heap.get(index)->priorityScore > heap.get(parent)->priorityScore) 
            {
                heap.swap(index, parent); 
                index = parent;
            } 
            else 
                break;
        }
    }
    
    void heapifyDown(int index) 
    {
        int left = 2 * index + 1, right = 2 * index + 2, largest = index;
        
        if (left < heap.size() && heap.get(left)->priorityScore > heap.get(largest)->priorityScore) 
            largest = left;
        if (right < heap.size() && heap.get(right)->priorityScore > heap.get(largest)->priorityScore) 
            largest = right;
        if (largest != index) 
        { 
            heap.swap(index, largest);
            heapifyDown(largest);
        }
    }

public:
    void insert(Parcel* val) 
    { 
        heap.add(val); 
        heapifyUp(heap.size() - 1); 
    }
    
    Parcel* extractMax() 
    {
        if (heap.isEmpty()) 
            return nullptr;
        
        Parcel* maxVal = heap.get(0);
        heap.set(0, heap.get(heap.size() - 1));
        heap.removeLast();
        if (!heap.isEmpty()) 
            heapifyDown(0);
        
        return maxVal;
    }
    
    bool isEmpty() 
    { 
        return heap.isEmpty(); 
    }
    
    int size() 
    {
        return heap.size();
    }
};

struct HashEntry 
{ 
    string key; 
    Parcel* value; 
    bool occupied; 
    HashEntry() : key(""), value(nullptr), occupied(false) {} 
};

class ParcelHashTable 
{
private:
    HashEntry* table; 
    int capacity;
    int hashFunction(const string& key) 
    {
        unsigned long hash = 5381;
        for (char c : key) 
            hash = ((hash << 5) + hash) + c;
        return hash % capacity;
    }

public:
    ParcelHashTable(int cap = 1007) : capacity(cap) 
    { 
        table = new HashEntry[capacity]; 
    }
    
    ~ParcelHashTable() 
    {
        delete[] table; 
    }
    
    void insert(string key, Parcel* value) 
    {
        int index = hashFunction(key); 
        int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;
            if (!table[probe].occupied || table[probe].key == key) 
            {
                table[probe].key = key; 
                table[probe].value = value; 
                table[probe].occupied = true; 
                return;
            }
            i++;
        }
    }
    
    Parcel* search(string key) 
    {
        int index = hashFunction(key); 
        int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;

            if (!table[probe].occupied) 
                return nullptr;
            
            if (table[probe].key == key) 
                return table[probe].value;
            i++;
        }
        return nullptr;
    }
    
    void printAll() 
    {
        clearScreen();
        cout << "\n=========================================\n           SYSTEM DATABASE\n=========================================\n";
        cout << "ID     DEST         CAT     ZONE     STATUS\n";
        for(int i=0; i<capacity; i++) 
            if(table[i].occupied) 
                cout << *table[i].value << endl;
        cout << "-----------------------------------------\n";
    }
};

struct UndoAction 
{ 
    string type; 
    string parcelId; 
};

struct UndoNode 
{ 
    UndoAction data; 
    UndoNode* next; 
    UndoNode(UndoAction d) : data(d), next(nullptr) {} 
};

class ActionStack 
{
private:
    UndoNode* topNode;

public:
    ActionStack() : topNode(nullptr) {}
    
    void push(string type, string id) 
    { 
        UndoNode* n = new UndoNode({type, id}); 
        n->next = topNode; 
        topNode = n; 
    }
    
    bool pop(UndoAction& dest) 
    {
        if(!topNode) 
            return false;
        
        UndoNode* temp = topNode; 
        dest = temp->data; 
        topNode = topNode->next; 
        delete temp; 
        
        return true;
    }
};

class RiderManager {
private:
    Rider* riders;
    int riderCount;
    
public:
    RiderManager() : riderCount(4) {
        riders = new Rider[4] {
            Rider("Ali Khan", "Light", 50),
            Rider("Bilal Ahmed", "Heavy", 200),
            Rider("Usman Tariq", "Priority", 100),
            Rider("Fahad Mustafa", "General", 150)
        };
    }
    
    ~RiderManager() {
        delete[] riders;
    }
    
    string assignRider(Parcel* parcel) {
        int bestRider = -1;
        int bestScore = -1;
        
        for(int i = 0; i < riderCount; i++) {
            if(riders[i].available) {
                int score = 0;
                
                if (parcel->priority == 3 && riders[i].type == "Priority") {
                    score += 100;
                }
                
                if (parcel->weightCategory == "Heavy" && riders[i].type == "Heavy") {
                    score += 80;
                } else if (parcel->weightCategory == "Light" && riders[i].type == "Light") {
                    score += 80;
                }
                
                int loadPercentage = (riders[i].currentLoad * 100) / riders[i].maxLoad;
                score += (100 - loadPercentage);
                
                if (score > bestScore) {
                    bestScore = score;
                    bestRider = i;
                }
            }
        }
        
        if (bestRider != -1) {
            riders[bestRider].currentLoad += parcel->weight;
            riders[bestRider].parcelsAssigned++;
            if (riders[bestRider].currentLoad >= riders[bestRider].maxLoad * 0.9) {
                riders[bestRider].available = false;
            }
            return riders[bestRider].name + " (" + riders[bestRider].type + ")";
        }
        
        return "";
    }
    
    void releaseRider(string riderName) {
        for(int i = 0; i < riderCount; i++) {
            if (riders[i].name == riderName.substr(0, riderName.find(" ("))) {
                riders[i].currentLoad = max(0, riders[i].currentLoad - 10);
                if (riders[i].currentLoad < riders[i].maxLoad * 0.7) {
                    riders[i].available = true;
                }
                break;
            }
        }
    }
    
    void showRiderStatus() {
        clearScreen();
        cout << "\n=== RIDER STATUS ===\n";
        cout << left << setw(20) << "Name" << setw(10) << "Type" 
             << setw(10) << "Load" << setw(10) << "Max" 
             << setw(10) << "Parcels" << setw(0) << "Status\n";
        for(int i = 0; i < riderCount; i++) {
            cout << setw(20) << riders[i].name 
                 << setw(10) << riders[i].type
                 << setw(10) << riders[i].currentLoad 
                 << setw(10) << riders[i].maxLoad
                 << setw(10) << riders[i].parcelsAssigned
                 << setw(10) << (riders[i].available ? "Available" : "Busy") << endl;
        }
    }
};

class LogisticsEngine 
{
private:
    ParcelHashTable database;
    ParcelHeap sortingQueue;
    ParcelLinkedList shippingList;
    RiderManager riderManager;
    MapGraph map;
    ActionStack undoStack;
    
public:
    LogisticsEngine() 
    {
        srand(time(0));
        setupMap();
    }

    void setupMap() 
    {
        int lhr = map.addCity("Lahore", "Zone A");
        int isb = map.addCity("Islamabad", "Zone B");
        int khi = map.addCity("Karachi", "Zone C");
        int pew = map.addCity("Peshawar", "Zone B");
        int mux = map.addCity("Multan", "Zone A");
        int fsd = map.addCity("Faisalabad", "Zone A");
        int qta = map.addCity("Quetta", "Zone D");
        int rwp = map.addCity("Rawalpindi", "Zone B");
        int suk = map.addCity("Sukkur", "Zone C");
        int sgd = map.addCity("Sargodha", "Zone A");
        int bwp = map.addCity("Bahawalpur", "Zone D");
        int guj = map.addCity("Gujranwala", "Zone A");
        int swl = map.addCity("Sahiwal", "Zone A");
        int okk = map.addCity("Okara", "Zone B");
        int mbd = map.addCity("Mandi Bahauddin", "Zone C");
        int ryk = map.addCity("Rahim Yar Khan", "Zone D");
        int hfd = map.addCity("Hafizabad", "Zone A");
        int skt = map.addCity("Sialkot", "Zone A");
        int ghr = map.addCity("Gujrat", "Zone A");
        int veh = map.addCity("Vehari", "Zone D");
        int goj = map.addCity("Gojra", "Zone A");
        int mrd = map.addCity("Mardan", "Zone B");
        int swb = map.addCity("Swabi", "Zone B");
        int gwd = map.addCity("Gwadar", "Zone D");
        int hyd = map.addCity("Hyderabad", "Zone C");
        
        map.addRoad(lhr, isb, 375, 20);
        map.addRoad(lhr, fsd, 180, 18);
        map.addRoad(lhr, mux, 345, 15);
        map.addRoad(lhr, guj, 70, 15);
        map.addRoad(lhr, swl, 175, 12);
        map.addRoad(lhr, okk, 100, 10);        
        map.addRoad(isb, pew, 155, 18);
        map.addRoad(isb, rwp, 20, 25);       
        map.addRoad(fsd, sgd, 85, 15);
        map.addRoad(fsd, goj, 30, 10);
        map.addRoad(fsd, mux, 240, 15);        
        map.addRoad(mux, bwp, 90, 12);
        map.addRoad(mux, veh, 70, 10);
        map.addRoad(mux, suk, 490, 15);        
        map.addRoad(suk, khi, 470, 18);
        map.addRoad(suk, hyd, 320, 15);
        map.addRoad(suk, qta, 390, 12);        
        map.addRoad(khi, hyd, 165, 20);
        map.addRoad(khi, gwd, 700, 10);        
        map.addRoad(qta, gwd, 700, 10);        
        map.addRoad(pew, mrd, 50, 15);
        map.addRoad(pew, swb, 70, 12);        
        map.addRoad(mrd, swb, 40, 12);        
        map.addRoad(guj, skt, 50, 15);
        map.addRoad(guj, ghr, 40, 15);
        map.addRoad(guj, hfd, 50, 12);        
        map.addRoad(skt, ghr, 60, 12);        
        map.addRoad(sgd, mbd, 110, 12);        
        map.addRoad(bwp, ryk, 80, 12);       
        map.addRoad(swl, okk, 50, 12);
        map.addRoad(swl, veh, 120, 10);      
        map.addRoad(hfd, lhr, 105, 15);
        map.addRoad(hfd, fsd, 90, 12);       
        map.addRoad(veh, bwp, 140, 10);       
        map.addRoad(hyd, gwd, 550, 10);
    }

    void displayAllCities() {
        map.displayAllCities();
    }
    
    void requestPickup(string id, string dest, double w, int p) 
    {
        clearScreen();
        if (map.getCityIndex(dest) == -1) 
        { 
            cout << "Error: Destination not valid.\n"; 
            pauseFunc(); 
            return; 
        }
        
        if (database.search(id)) 
        {
            cout << "Error: ID already exists.\n"; 
            pauseFunc(); 
            return; 
        }
        
        string zone = map.getZone(dest);
        Parcel* newP = new Parcel(id, dest, w, p, zone);
        database.insert(id, newP); 
        
        newP->updateStatus(STATUS_WAREHOUSE, "Arrived at Warehouse", "Central Hub");
        
        sortingQueue.insert(newP); 
        
        undoStack.push("ADD", id);
        cout << ">> Pickup Request Logged.\n";
        cout << "   Category: " << newP->weightCategory << " | Zone: " << newP->zone << "\n";
        cout << "   Status: Moved to Warehouse Queue.\n";
        pauseFunc();
    }

    void processNext() 
    {
        clearScreen();
        if(sortingQueue.isEmpty()) 
        { 
            cout << ">> Warehouse Queue Empty.\n"; 
            pauseFunc(); 
            return; 
        }

        Parcel* p = sortingQueue.extractMax(); 
        string rider = riderManager.assignRider(p);
        
        if (rider.empty()) {
            cout << ">> No Suitable Riders Available! Parcel returned to queue.\n";
            sortingQueue.insert(p);
            pauseFunc();
            return;
        }
        
        p->assignedRider = rider;
        
        int start = map.getCityIndex("Lahore");
        int end = map.getCityIndex(p->destination);

        cout << ">> Calculating routes for " << p->id << " to " << p->destination << "...\n";
        map.findAllPaths(start, end);

        if (map.pathCount == 0) 
        {
            cout << ">> ALERT: No paths available. Returning to Sender.\n";
            p->updateStatus(STATUS_RETURNED, "No Route Available", "Warehouse");
            riderManager.releaseRider(rider);
            pauseFunc();
            return;
        }

        int minIdx = map.getMinRouteIndex();
        cout << "------------------------------------------------\n";
        for(int i=0; i<map.pathCount; i++) 
        {
            cout << "   [" << i << "] Distance: " << map.availablePathDistances[i] << " km ";
            if(i == minIdx) cout << "(RECOMMENDED - SHORTEST)";
            cout << "\n       Path: ";
            IntArrayList& path = map.availablePaths[i];
            for(int j=0; j<path.size(); j++) 
            {
                cout << map.cities[path.get(j)].name << (j < path.size()-1 ? " -> " : "");
            }
            cout << "\n";
        }
        cout << "------------------------------------------------\n";
        
        int choice;
        cout << ">> Select Route ID to Dispatch: ";
        cin >> choice;
        
        if (choice < 0 || choice >= map.pathCount) 
        {
            cout << "Invalid selection. Using Recommended Route.\n";
            choice = minIdx;
        }

        if (rand() % 10 < 3)
        { 
            if (rand() % 2 == 0) {
                cout << "\n>>> [LIVE UPDATE] Road Blockage Detected on selected route!\n";
                map.blockRandomRoad();
            } else {
                cout << "\n>>> [LIVE UPDATE] Road Overload Detected on selected route!\n";
                map.checkOverloads();
            }
            
            cout << ">>> Re-calculating Best Route automatically...\n";
            map.findAllPaths(start, end); 
            
            if (map.pathCount > 0) 
            {
                choice = map.getMinRouteIndex(); 
                cout << ">>> Rerouted to new optimal path.\n";
            } 
            else 
            {
                 cout << ">>> Rerouting Failed. Returning to Sender.\n";
                 p->updateStatus(STATUS_RETURNED, "Dynamic Route Issue - RTS", "Warehouse");
                 riderManager.releaseRider(rider);
                 pauseFunc();
                 return;
            }
        }

        p->currentRoute = new IntArrayList(map.availablePaths[choice]);
        p->currentRouteDistance = map.availablePathDistances[choice];
        p->currentPosition = 0;
        
        map.updateEdgeLoad(map.availablePaths[choice], 1);

        p->updateStatus(STATUS_LOADING, "Loading onto Truck", "Bay 4");
        long long travelSecs = 10 + (rand() % 21);
        p->dispatchTime = time(0);
        p->arrivalTime = time(0) + travelSecs + 5; 
        p->lastKnownTime = time(0);
        
        shippingList.pushBack(p);
        undoStack.push("DISPATCH", p->id);
        
        cout << "\n>> PARCEL DISPATCHED SUCCESSFULLY.\n";
        cout << "   Rider: " << rider << "\n   ETA: " << travelSecs << "s (transit) + 5s (loading)\n";
        cout << "   Route Distance: " << p->currentRouteDistance << " km\n";
        
        pauseFunc();
    }

    void showMap() 
    { 
        map.displayNetwork(); 
        pauseFunc(); 
    }
    
    void showRiderStatus() {
        riderManager.showRiderStatus();
        pauseFunc();
    }

    void undoLast() 
    {
        clearScreen();
        UndoAction act;
        if(undoStack.pop(act)) 
        {
            Parcel* p = database.search(act.parcelId);
            if(p) 
            {
                if(act.type == "ADD") 
                {
                    p->updateStatus(STATUS_CANCELLED, "Undo: Creation Reverted", "N/A");
                    cout << ">> Undo Add: Parcel " << p->id << " marked as cancelled.\n";
                } 
                else if (act.type == "DISPATCH") 
                {
                    p->updateStatus(STATUS_WAREHOUSE, "Undo: Dispatch Reverted", "Warehouse");
                    p->arrivalTime = 0; 
                    
                    if (!p->assignedRider.empty()) {
                        riderManager.releaseRider(p->assignedRider);
                    }
                    if (p->currentRoute) {
                        map.updateEdgeLoad(*p->currentRoute, -1);
                    }
                    
                    sortingQueue.insert(p); 
                    cout << ">> Undo Dispatch: Parcel " << p->id << " returned to warehouse.\n";
                }
            }
        } 
        else 
        { 
            cout << ">> Nothing to undo.\n"; 
        }
        
        pauseFunc();
    }

    void updateRealTime() 
    {
        long long now = time(0);
        
        if (now % 10 == 0) {
            map.checkOverloads();
        }
        
        shippingList.recalculateRoutes(&map);
        
        shippingList.updateLifecycle(now, &map);
    }
    
    void liveMonitor() 
    {
        char cmd = 'r';
        while (cmd == 'r' || cmd == 'R') 
        {
            clearScreen();
            long long now = time(0);
            updateRealTime();
            shippingList.showTransitStatus(now);
            cout << "\n[r] Refresh View   [x] Exit to Menu\nSelect: ";
            cin >> cmd;
        }
    }

    void viewParcel(string id) 
    {
        clearScreen();
        Parcel* p = database.search(id);
        if(p) 
        {
            cout << "\n==============================\n    PARCEL TRACKING DETAILS   \n==============================\n";
            cout << *p << endl;
            cout << "Assigned Rider: " << (p->assignedRider.empty() ? "Not Assigned" : p->assignedRider) << endl;
            if (p->currentRoute) {
                cout << "Route Distance: " << p->currentRouteDistance << " km\n";
                cout << "Current Position: " << p->currentPosition << "/" << p->currentRoute->size() << endl;     
            }
            p->history->printTimeline();
            
            if (p->status == STATUS_IN_TRANSIT || p->status == STATUS_LOADING) 
            {
                long long now = time(0);
                long long rem = p->arrivalTime - now;
                if(rem > 0) cout << "\n>>> EST. DELIVERY IN: " << rem << " seconds\n";
            }
        } 
        else 
        { 
            cout << "Parcel Not Found.\n"; 
        }
        
        pauseFunc();
    }

    void listAll() 
    { 
        database.printAll(); 
        pauseFunc(); 
    }
    
    void cancelParcel(string id) 
    {
        clearScreen();
        Parcel* p = database.search(id);
        if(p && p->status <= STATUS_WAREHOUSE) 
        {
            p->updateStatus(STATUS_CANCELLED, "Cancelled by User", "Warehouse");
            cout << ">> Parcel " << id << " cancelled.\n";
        } 
        else if (p && p->status == STATUS_IN_TRANSIT) {
            cout << ">> Parcel is in transit. Attempting to reroute back...\n";
            p->updateStatus(STATUS_RETURNED, "Cancelled During Transit - Returning", "In Transit");
            if (p->currentRoute) {
                map.updateEdgeLoad(*p->currentRoute, -1);
            }
        }
        else 
            cout << ">> Error: Cannot cancel (Already Delivered or Missing).\n";
        
        pauseFunc();
    }
    
    void forceRouteRecalculation() {
        clearScreen();
        cout << ">>> Forcing route recalculation for all parcels in transit...\n";
        shippingList.recalculateRoutes(&map);
        cout << ">>> Route recalculation complete.\n";
        pauseFunc();
    }
};

int main() 
{
    LogisticsEngine engine;
    int mainChoice;
    
    while (true) 
    {
        clearScreen();
        cout << "\n===== SWIFT-EX MAIN CONTROL PANEL ======\n";
        cout << "= 1. Intelligent Parcel Sorting Module =\n";
        cout << "= 2. Parcel Routing Module             =\n";
        cout << "= 3. Parcel Tracking System            =\n";
        cout << "= 4. Courier Operations Engine         =\n";
        cout << "= 5. Exit System                       =\n";
        cout << "========================================\n";
        cout << "\nSelect Module: ";
        
        if (!(cin >> mainChoice)) 
        { 
            cin.clear(); 
            cin.ignore(1000, '\n'); 
            continue; 
        }
        if (mainChoice == 5) break;

        switch(mainChoice) 
        {
            case 1: 
            {
                int sub = 0;
                while (sub != 4) {
                    clearScreen();
                    cout << "\n---  INTELLIGENT PARCEL SORTING MODULE ---\n";
                    cout << "1. New Pickup Request\n";
                    cout << "2. Cancel/Withdraw Parcel\n";
                    cout << "3. View Sorting Database\n";
                    cout << "4. Return to Main Menu\n";
                    cout << "Enter choice: ";
                    cin >> sub;
                    if(sub == 1) {
                        string id, dest; double w; int p;
                        cout << "ID: "; cin >> id; cin.ignore(1000, '\n');
                        engine.displayAllCities();
                        cout << "Destination: "; getline(cin, dest);
                        cout << "Weight: "; cin >> w;
                        cout << "Priority (1=Normal, 2=Express, 3=Overnight): "; 
                        cin >> p;
                        engine.requestPickup(id, dest, w, p);
                    } 
                    else if(sub == 2) { 
                        string id; 
                        cout << "ID to Cancel: "; 
                        cin >> id; 
                        engine.cancelParcel(id); 
                    }
                    else if(sub == 3) { 
                        engine.listAll(); 
                    }
                    engine.updateRealTime();
                }
                break;
            }
            case 2:
            {
                int sub = 0;
                while (sub != 3) {
                    clearScreen();
                    cout << "\n--- PARCEL ROUTING MODULE ---\n";
                    cout << "1. View Map & Network Status\n";
                    cout << "2. Force Route Recalculate\n";
                    cout << "3. Return to Main Menu\n";
                    cout << "Enter choice: ";
                    cin >> sub;
                    if(sub == 1) engine.showMap();
                    else if(sub == 2) engine.forceRouteRecalculation();
                    engine.updateRealTime();
                }
                break;
            }
            case 3:
            {
                int sub = 0;
                while (sub != 3) {
                    clearScreen();
                    cout << "\n--- PARCEL TRACKING SYSTEM ---\n";
                    cout << "1. Track Specific Parcel\n";
                    cout << "2. Live Transit Monitor\n";
                    cout << "3. Return to Main Menu\n";
                    cout << "Ente choice: ";
                    cin >> sub;
                    if(sub == 1) { 
                        string id; 
                        cout << "Enter Parcel ID: "; 
                        cin >> id; 
                        engine.viewParcel(id); 
                    }
                    else if(sub == 2) engine.liveMonitor();
                    engine.updateRealTime();
                }
                break;
            }
            case 4:
            {
                int sub = 0;
                while (sub != 4) {
                    clearScreen();
                    cout << "\n--- COURIER OPERATIONS ENGINE ---\n";
                    cout << "1. Warehouse Dispatch (Process Next)\n";
                    cout << "2. View Rider Status\n";
                    cout << "3. Undo Last Operation\n";
                    cout << "4. Return to Main Menu\n";
                    cout << "Enter choice: ";
                    cin >> sub;
                    if(sub == 1) engine.processNext();
                    else if(sub == 2) engine.showRiderStatus();
                    else if(sub == 3) engine.undoLast();
                    engine.updateRealTime();
                }
                break;
            }
        }
        engine.updateRealTime();
    }
    return 0;
}