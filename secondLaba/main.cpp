#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <limits>
#include <numeric>
#include <algorithm>

using namespace std;


void findMinMax(const vector<int>& arr, int& min_val, int& max_val) {
    min_val = numeric_limits<int>::max();
    max_val = numeric_limits<int>::min();

    for (const int& num : arr) {
        if (num < min_val) min_val = num;
        this_thread::sleep_for(chrono::milliseconds(7));  
        if (num > max_val) max_val = num;
        this_thread::sleep_for(chrono::milliseconds(7));  
    }

    cout << "Min value: " << min_val << endl;
    cout << "Max value: " << max_val << endl;
}

void findAverage(const vector<int>& arr, int& avg) {
    int sum = 0;

    for (const int& num : arr) {
        sum += num;
        this_thread::sleep_for(chrono::milliseconds(12)); 
    }
    double ans = double(sum) / arr.size();
    avg = int(ans);
    cout << "Average: " << ans << endl;
}

int main() {
    int n;
    cout << "Enter the number of elements in the array: ";
    cin >> n;

    vector<int> arr(n);
    cout << "Enter " << n << " integers:" << endl;
    for (int i = 0; i < n; ++i) {
        cin >> arr[i];
    }

    int min_val, max_val;
    int avg;

    thread min_max_thread(findMinMax, ref(arr), ref(min_val), ref(max_val));
    thread avg_thread(findAverage, ref(arr), ref(avg));

    min_max_thread.join();
    avg_thread.join();

    replace(arr.begin(), arr.end(), min_val, avg);
    replace(arr.begin(), arr.end(), max_val, avg);

    cout << "Modified array: ";
    for (const int& x : arr) {
        cout << x << " ";
    }
    cout << endl;

    return 0;
}