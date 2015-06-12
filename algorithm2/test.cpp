#include<vector>

using namespace std;

int main() {
	vector<int> test;
	test.push_back(1);
	test.push_back(2);
	test.push_back(3);
	test.push_back(4);

	vector<int>::iterator iter = test.begin();
	
	test.erase(iter);
	test.erase(iter);
	test.erase(iter);
	test.erase(iter);
}
