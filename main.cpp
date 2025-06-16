//Yigit Demirkan
#include <iostream>
#include <thread>
#include <random>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <vector>
using namespace std;


mutex myMutex;
mutex coutMutex;
int ctr;
int targetnumber;
vector <bool> finding;
vector <int> counting;
bool isOver = false;
struct tm *time1;
struct tm *time2;


//Choosing a random number between min and max without using RandGen
int random_range(const int & min, const int & max)
{
    static mt19937 generator(time(0));
    uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}

//Entry point for the host
void fhost(int min, int max, int num_rounds){
    int iterator = 0;
    while (iterator < num_rounds){ //Iterate for each round
        time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
        tm *ptm = new struct tm;
        localtime_s(ptm, &tt);
        if (iterator == 0){ //If we're at the first round
            coutMutex.lock();
            cout << "---------------------------------------------------" << endl;
            cout << "Game started at: " << put_time(ptm,"%X")<<endl;
            cout << "Round 1 will start 3 seconds later" <<endl;
            cout << endl;
            coutMutex.unlock();
        }
        else{
            coutMutex.lock();
            cout << endl;
            cout << "---------------------------------------------------" << endl;
            cout << "Round " << iterator+1 << " started at: " <<  put_time(ptm,"%X") << endl;
            coutMutex.unlock();
        }
        targetnumber = random_range(min,max); // Setting the target for the current round
        coutMutex.lock();
        cout << "Target is " << targetnumber << endl;
        cout << endl;
        coutMutex.unlock();
        if (iterator == 0){ //If it is first round, sleep until time1, which is 3 seconds after current time, defined in the function for players
            this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(time1)));
        }
        myMutex.lock();
        while (!isOver) { //Do not start the next round without getting a correct guess
            myMutex.unlock();
            this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(time2)));
            myMutex.lock();
        }
        ctr++; //Increment the global counter which is used by player function
        myMutex.unlock();

        for (int k=0; k < finding.size() ; k++){ //Updating the scores
            if (finding[k]){
                counting[k] = counting[k]+1;
                finding[k] = false;
            }
        }
        if(isOver&&(iterator!=num_rounds-1)){ //making the control flag default again
            isOver = false;

        }


        iterator++;
    }
}

//Entry point for the players
void players(int order, int min, int max,int num_rounds){
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    time1 = localtime(&tt);
    time1->tm_sec=time1->tm_sec+3;
    this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(time1))); //Sleep for 3 seconds at the first round
    myMutex.lock();
    while (ctr < num_rounds){ //Iterate for each round
        while (!isOver){ //Until one of the players make a correct guess
            int guess = random_range(min,max);
            if (guess == targetnumber){
                time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
                tm *ptm = new struct tm;
                localtime_s(ptm, &tt);
                finding[order] = true; //We use this information while updating score at the host thread
                coutMutex.lock();
                cout << "Player with id " << order << " guessed " << guess << " correctly at: " << put_time(ptm,"%X")<<endl;
                coutMutex.unlock();
                isOver = true;
            }
            else {
                time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
                tm *ptm = new struct tm;
                localtime_s(ptm, &tt);
                coutMutex.lock();
                cout << "Player with id " << order << " guessed " << guess << " incorrectly at: " << put_time(ptm, "%X")<< endl;
                coutMutex.unlock();
            }
            myMutex.unlock();
            this_thread::sleep_for (chrono::seconds(1)); //1 second sleep after guessing
            time2 = localtime(&tt); //It will be used by host thread
            myMutex.lock();
        }
        myMutex.unlock();
        myMutex.lock();

    }
    myMutex.unlock();
}

int main(){
    int num_players, num_rounds, min, max;
    cout << "Please enter number of players" << endl;
    cin >> num_players;
    while(num_players < 1){ //Taking the number of players as input and checking it
        cout << "Number of players cannot be lower than 1!" << endl;
        cout << "Please enter number of players" << endl;
        cin >> num_players;
    }

    cout << "Please enter number of rounds" << endl;
    cin >> num_rounds;
    while(num_rounds < 1){//Taking the number of rounds as input and checking it
        cout << "Number of rounds cannot be lower than 1!" << endl;
        cout << "Please enter number of rounds" << endl;
        cin >> num_rounds;
    }

    cout << "Please enter the randomization range" << endl;
    cin >> min >> max;
    while(min > max){//Taking the randomization range as input and checking it
        cout << "Lower bound has to be smaller than or equal to higher bound" << endl;
        cout << "Please enter the randomization range" << endl;
        cin >> min >> max;
    }
    vector <bool> temp(num_players); //A temporary vector for determining the finding vector
    for (int k=0; k< num_players; k++){
        temp[k] = false;
    }
    finding = temp;

    vector <int> temp2(num_players);//A temporary vector for determining the counting vector
    for (int z=0; z< num_players; z++){
        temp2[z] = 0;
    }
    counting = temp2;

    ctr = 0;
    thread threads[num_players];
    thread host;
    isOver = false;
    host = thread(&fhost,min,max,num_rounds);
    for (int k = 0; k < num_players; k++){
        threads[k] = (thread(&players, k, min, max,num_rounds));
    }
    host.join();
    for (int i=0; i < num_players; i++){
        threads[i].join();
    }
    cout << endl;
    cout << "Game is over!"<< endl;
    cout << "Leaderboard:" << endl;
    for (int a=0; a < counting.size() ; a++){ //Displaying the scores
        cout << "Player " << a << " has won " << counting[a] << " times" << endl;
    }
    return 0;
}