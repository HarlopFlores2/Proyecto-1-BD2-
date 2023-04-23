#include "sequentialFile.h"
#include <unistd.h>

int main(){
    sequentialFile SequentialFile(100);

    //SequentialFile.load_data("../file_out.csv");
    SequentialFile.readRecord(33355);
    cout << SequentialFile.findLocation(36).second << endl;


}