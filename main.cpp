#include<iostream>
#include"main.h"
using namespace std;
int main() {
    FaultHandler f("error.txt");
    f.debugOn();
    f.test();
    cin.get();
}