#include <stdio.h>
#include <unistd.h>
int main(){
    char *x = "foo";
     
    int returnvalue= fork();
    if (returnvalue == 0){
        x[0] = 'b';
        printf("child");
    }
    else {
        
        printf("parent: %s\n", x);
        
    }
    return 0;
}