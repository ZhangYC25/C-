#include <vector>
#include <iostream>


//bubble sort

void bubble(std::vector<int> &nums){
    int n = nums.size() - 1;
    for (int i = 0;i <= n;++i) {
        for (int j = 0;j <= n - i - 1;++j){
            if (nums[j] > nums[j + 1]){
                std::swap(nums[j], nums[j + 1]);
            }
        }
    }
}

//select sort
void select(std::vector<int> &nums){
    int n = nums.size();

    for (int i = 0;i < n - 1;++i){
        int min = i;
        for (int j = i;j < n;++j){
            if(nums[j] < nums[min]){
                min = j;
            }
        }
        std::swap(nums[i], nums[min]);
    }
}

//insert sort
void insert(std::vector<int> &nums){

    int n = nums.size();
    for(int i = 1; i < n; ++i){
        int tmp = nums[i];
        int index = i;
        for (int j = i - 1;j >= 0;--j){
            if (nums[j] > tmp) {
                nums[j+1] = nums[j];
                index = j;
            }
        }
        nums[index] = tmp;
    }
}


//merge sort
// right = size()-1
void merge(std::vector<int> &nums, int left, int mid, int right){
    
    std::vector<int> tmp(nums.size());
    int l_pos = left, r_pos = mid+1;
    int pos = left;
    while (l_pos <= mid && r_pos <= right)
    {
        if (nums[l_pos] < nums[r_pos]) {
            tmp[pos] = nums[l_pos];
            ++l_pos;
            ++pos;
        }
        if (nums[l_pos] >= nums[r_pos]) {
            tmp[pos] = nums[r_pos];
            ++r_pos;
            ++pos;
        }
    }

    while(l_pos <= mid){
        tmp[pos] = nums[l_pos];
        ++pos;
        ++l_pos;
    }

    while(r_pos <= right){
        tmp[pos] = nums[r_pos];
        ++pos;
        ++r_pos;
    }

    while(left <= right){
        nums[left] = tmp[left];
        left ++;
    }
}



void mergesort(std::vector<int> &nums, int left, int right){
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergesort(nums, left, mid);
        mergesort(nums, mid+1, right);
        merge(nums, left, mid, right);
    }
}

//quick sort

void quicksort(std::vector<int> &nums, int left, int right){

    if (left > right) return;

    int tmp = nums[left];

    int l = left, r = right;

    while(l < r){
        while(l<r && nums[r] >= tmp) r--;
        while(l<r && nums[l] <= tmp) l++;

        if(l<r) std::swap(nums[l], nums[r]);
    }
    if(r != left) std::swap(nums[left], nums[r]);
    quicksort(nums, left, r - 1);
    quicksort(nums, r + 1, right);
}

//heap sort
//max_size = size()
void heapify(std::vector<int> &nums, int max_size, int last_leaf){
    int l = last_leaf * 2 + 1;
    int r = last_leaf * 2 + 2;

    if (l >= max_size || r >= max_size) return;

    int large = last_leaf;
    if (nums[large] < nums[l]) large = l;
    if (nums[large] < nums[r]) large = r;
    if (last_leaf != large){
        std::swap(nums[last_leaf], nums[large]);
        heapify(nums, max_size, large);
    }
}

void heapsort(std::vector<int> &nums){
    int n = nums.size();
    for (int i = n/2 - 1;i >= 0;--i) {
        heapify(nums, n, i);
    }

    for (int i = n - 1;i >= 0;--i) {
        std::swap(nums[0], nums[i]);
        heapify(nums, i, 0);
    }
}

int main(){
    std::vector<int> arr = {61,17,29,22,34,60,73,56,89,12};

    //bubble sort
    //bubble(arr);


    //select sort
    //select(arr);


    //insert sort
    //insert(arr);



    //merge sort
    int n = arr.size();
    std::vector<int> tmp(n);
    //mergesort(arr, 0, n-1);

    //quick sort
    //quicksort(arr, 0, n-1);

    //heap sort
    heapsort(arr);
    for (int i = 0;i < n;++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}