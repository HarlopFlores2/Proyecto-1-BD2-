#include "sequentialFile.h"


int main(){
    sequentialFile SequentialFile(100);
    SequentialFile.load_data("../file_out.csv");
}