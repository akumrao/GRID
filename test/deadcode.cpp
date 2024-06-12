#include <stdio.h>


class deadcode
{
public:
	deadcode();
	~deadcode();

	void fn()
	{
		puts("This is dead code which is never called.");

	}

	void fn1()
	{
		puts("This is dead code which is never called.");

	}
	
};


void unused(){
	puts("This is dead code which is never called.");
}

int main(){

	deadcode dc;

	dc.fn();


	return 0;
}



/*

#remove dead code 
cppcheck
https://softwareengineering.stackexchange.com/questions/157774/how-do-i-locate-uncalled-functions

https://stackoverflow.com/questions/6687630/how-to-remove-unused-c-c-symbols-with-gcc-and-ld

https://tetzank.github.io/posts/removing-unused-code/#:~:text=Instead%20of%20changing%20the%20file,a%20special%20flag%20of%20GCC.&text=%2Dfwhole%2Dprogram%20lets%20the%20compiler,()%20without%20any%20source%20changes.


*/