/*Amna Aiman 21i-2743
Noor-ul-huda 21i-1357
Abu Bakar 21i-1691
*/
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>
#include <mutex>
#include <semaphore.h>
#include <ctime>
using namespace std;
//Data intialization
bool *syncFlags;
int recvArrSize;
double inbtwResult[4];
double layerResult[4];
const int numInp = 2;
const int maxInp = 4;
int hiddLayers = 3;
int numNeuron = 4;

double **inpLayer_Wieghts;
double ***neuronsWeights;
int initialInp[numInp] = {1, 2};

int **fd;
pthread_mutex_t *fdMutex;
sem_t *syncSemap;
pthread_mutex_t *semapMutex;
//the function which take account of neuron and layers
void* Neuron(void* arg) {
    int* args = (int*)arg;
    int currLayer = args[0];
    int currNeuron = args[1];
    double input[maxInp];

    if (currLayer == 0) {
        cout << "The current running Layer is zero" << endl;
        cout << "The value calculated for a neuron is: " << currNeuron << endl;

        double weightedSum = 0.0;
        for (int i = 0; i < numInp; i++) {
            weightedSum += (initialInp[i] * inpLayer_Wieghts[i][currNeuron]);
        }
        inbtwResult[currNeuron] = weightedSum;
        if (currNeuron == numNeuron - 1) {
            cout << "Calculating values: "<<endl;
            for (int i = 0; i < numNeuron; i++) {
                cout << inbtwResult[i] << " ";
            }
            cout << endl;

            int arrSize = sizeof(inbtwResult) / sizeof(inbtwResult[0]);
            cout << "Writing the answer of the first layer to the pipe" << endl;
            int fdWrite = currLayer;

            pthread_mutex_lock(&fdMutex[fdWrite]);
            syncFlags[currLayer] = true;
            if (write(fd[fdWrite][1], &arrSize, sizeof(arrSize)) == -1) {
                cout << "Error writing to pipe" << endl;
                return 0;
            }
            if (write(fd[fdWrite][1], inbtwResult, sizeof(inbtwResult)) == -1) {
                cout << "Error writing to pipe" << endl;
                return 0;
            }
            pthread_mutex_unlock(&fdMutex[fdWrite]);
            cout << "Done\n";
        }
    }
    if (currLayer != 0) {
        if (currNeuron == 0) {
            if (currLayer == 1) {
                sem_wait(&syncSemap[currLayer - 1]);

                sem_init(&syncSemap[1], 0, 1);
            }
            cout << "Waiting" << endl;
            cout << "Reading data from the pipe" << endl;

            int fdRead = currLayer - 1;
            pthread_mutex_lock(&fdMutex[fdRead]);
            if (read(fd[fdRead][0], &recvArrSize, sizeof(recvArrSize)) == -1) {
                cout << "Error reading from pipe" << endl;
                return 0;
            }
            double receivedArr[recvArrSize];
            if (read(fd[fdRead][0], receivedArr, sizeof(double) * recvArrSize) == -1) {
                cout << "Error reading from pipe" << endl;
                return 0;
            }

            pthread_mutex_unlock(&fdMutex[fdRead]);
            cout << "Received array: ";
            for (int i = 0; i < recvArrSize; i++) {
                cout << receivedArr[i] << " ";
                double acc = 0.0; // accumulator
                for (int j = 0; j < numNeuron; j++) {
                    acc += (receivedArr[j] * neuronsWeights[currLayer][currNeuron][i]);
                }

                layerResult[i] = acc;
            }
            cout << endl;
        }
        if (currNeuron == numNeuron - 1) {
            int arrSize = recvArrSize;
            int semValue;
            sem_getvalue(&syncSemap[currLayer], &semValue);
            cout<<"semaphore value "<<currLayer<<" is: "<<semValue<<"\n";
            cout << "Writing the answer to the pipe\n";



            int fdWrite = currLayer;
            pthread_mutex_lock(&fdMutex[fdWrite]);
            if (write(fd[fdWrite][1], &arrSize, sizeof(arrSize)) == -1) {
                cout << "Error writing to pipe" << endl;
                return 0;
            }
            if (write(fd[fdWrite][1], layerResult, sizeof(layerResult)) == -1) {
                cout << "Error writing to pipe" << endl;
                return 0;
            }
            pthread_mutex_unlock(&fdMutex[fdWrite]);

            cout << "Done" << endl;
        }
    }
    return NULL;
}
//formula of computing values
void computevalue(double finalValue)
{
//computeing values
        double q1 = 0.1;
        double q2 = 0.1;
        q1 = (((finalValue)*(finalValue)) + finalValue + 1) / 2;
        q2 = (((finalValue)*(finalValue)) - finalValue) / 2;
        initialInp[0] = q1;
        initialInp[1] = q2;

        cout<< initialInp[0]<<" "<<initialInp[1]<<"\n\n";
        }
//printing the welcome 
void printWelcomeMessage() {
     cout << "======================================" <<  endl;
     cout << "   Welcome to NeuralOS v1.0" <<  endl;
     cout << "======================================" <<  endl;
     cout << "Initializing neural networks..." <<  endl;
    cout << "======================================" <<  endl<<endl;
    cout<<"If You want to start the calculation Please Enter y(ie yes)"<<endl;
}

int main() {
char input;
printWelcomeMessage();
   cin>>input;
   system("clear");
   if(input=='y'||input=='Y')
   {
   cout<<"STARTING THE PROGRAM..."<<endl<<endl;
    neuronsWeights = new double**[hiddLayers];
    for (int i = 0; i < hiddLayers; i++) {
        neuronsWeights[i] = new double*[numNeuron];
        for (int j = 0; j < numNeuron; j++) {
            neuronsWeights[i][j] = new double[numNeuron];
        }
    }

    fd = new int*[hiddLayers];
    for (int i = 0; i < hiddLayers; i++) {
        fd[i] = new int[2];
    }
    syncFlags = new bool[hiddLayers];
    for (int i = 0; i < hiddLayers; i++) {
        syncFlags[i] = false;
    }




    semapMutex = new pthread_mutex_t[hiddLayers];
    syncSemap = new sem_t[hiddLayers];
    fdMutex = new pthread_mutex_t[hiddLayers];
    sem_init(&syncSemap[0], 0, 1);
    for (int i = 1; i < hiddLayers; i++) {
        pthread_mutex_init(&semapMutex[i], NULL);
        sem_init(&syncSemap[i], 0, 0);
    }

    int semValue;
    for (int i = 0; i < hiddLayers; i++) {
        sem_getvalue(&syncSemap[i], &semValue);
        cout<<"semaphore value is: "<<semValue<<endl;
    }

    inpLayer_Wieghts = new double*[numInp];
    for (int i = 0; i < numInp; i++) {
        inpLayer_Wieghts[i] = new double[numNeuron];
    }

    for (int i = 0; i < numInp; i++) {
        for (int j = 0; j < numNeuron; j++) {
            inpLayer_Wieghts[i][j] = ((double)rand() / RAND_MAX); // randomly assign
        }
    }

    for (int i = 0; i < hiddLayers; i++) {
        for (int j = 0; j < numNeuron; j++) {
            for (int k = 0; k < numNeuron; k++) {
                neuronsWeights[i][j][k] = ((double)rand() / RAND_MAX);
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int i = 0; i < hiddLayers; i++) {
            if (pipe(fd[i]) == -1) {
                cout << "Error making the pipe program ending" << endl;
                return 0;
            }
        }




        pid_t pid;
        int fdRead, fdWrite;

        for (int i = 0; i < hiddLayers; i++) {  // making layers
            cout << "Making process " << i << endl;
            pid = fork();

            if (pid == -1) {
                cout << "Fork failed. Program ending." << endl;
                return 0;
            }
            else if (pid == 0) {
               
                pthread_t threads[numNeuron];
                int args[numNeuron][2];
                for (int j = 0; j < numNeuron; j++) { // making threads
                    args[j][0] = i;
                    args[j][1] = j;
                    pthread_create(&threads[j], NULL, Neuron, (void*)args[j]);
                    cout << "Making thread " << j << endl;
                }
                for (int j = 0; j < numNeuron; j++) {
                    pthread_join(threads[j], NULL);
                }
                exit(0);
            }
            else {
                wait(NULL); // parent waits for child to finish
            }
        }

        cout << "***Reading final output" << endl;
        fdRead = hiddLayers - 1;
        cout << fdRead << endl;
        pthread_mutex_lock(&fdMutex[fdRead]);
        if (read(fd[fdRead][0], &recvArrSize, sizeof(recvArrSize)) == -1) {
            cout << "Error reading from pipe" << endl;
            return 0;
        }
        double receivedArr[recvArrSize];
        if (read(fd[fdRead][0], receivedArr, sizeof(double) * recvArrSize) == -1) {
            cout << "Error reading from pipe" << endl;
            return 0;
        }
        double finalValue = 0.0;
        pthread_mutex_unlock(&fdMutex[fdRead]);
        cout << "Received array: ";
        for (int i = 0; i < recvArrSize; i++) {
            cout << receivedArr[i] << " ";
            finalValue += receivedArr[i];
        }
        cout << finalValue;

        for (int i = hiddLayers - 1; i >= 0; i--) {
            cout << "Backtracking to layer " << i << endl;
            if (write(fd[i][1], &finalValue, sizeof(finalValue)) == -1) {
                cout << "Error writing to pipe" << endl;
                return 0;
            }
        }
        cout << "\nValue written to the first file" << endl;
        if (read(fd[0][0], &finalValue, sizeof(finalValue)) == -1) {
            cout<<"Error reading from pipe\n";
            return 0;
        }
        cout << finalValue << endl;
computevalue(finalValue);
        
    }
}
else 
    return 0;
}


