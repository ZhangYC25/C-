#include <iostream>
#include <vector>
template<typename T>
void bubble_sort(T arr[], int len){
    bool is_swap = false;
    for (int i = 0; i < len - 1; ++i) {
        for (int j = 0; j < len - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                is_swap = true;
            }
        }

        if (!is_swap) break;
    }
}

template<typename T>
void select_sort(T arr[], int len){
    bool is_swap = false;
    //int min_index = 0;
    for (int i = 0;i < len - 1;++i) {
        int min = i;
        for (int j = i;j < len;++j) {
            if (arr[j] < arr[min]) {
                min = j;
                //min_index = j;
                is_swap = true;
            }
        }
        if (!is_swap) break;
        std::swap(arr[i], arr[min]);
    }
}


template<typename T>
void insert_sort(T arr[], int len){
    for (int i = 1;i < len;++i) {
        int j = i - 1;
        int tem = arr[i];
        while (j >= 0 && tem < arr[j]) {
            arr[j + 1] = arr [j];
            --j;
        }
        arr[j+1] = tem;
    }
}


// merge sort
template<typename T>
void merge(std::vector<T>& nums, std::vector<T>& tmp, int left, int mid, int right){
    int l_pos = left, r_pos = mid + 1;
    int pos = left;

    while (l_pos <= mid && r_pos <= right) {
        if (nums[l_pos] < nums[r_pos]) {
            tmp[pos] = nums[l_pos];
            l_pos++;
            pos++;
        } else{
            tmp[pos] = nums[r_pos];
            r_pos++;
            pos++;
        }
    }

    while (l_pos <= mid) {
        tmp[pos] = nums[l_pos];
        pos++;
        l_pos++;
    }

    while (r_pos <= right) {
        tmp[pos] = nums[r_pos];
        pos++;
        r_pos++;
    }

    while (left <= right) {
        nums[left] = tmp[left];
        left++;
    }
}

template<typename T>
void partition(std::vector<T> &nums, std::vector<T>& tmp, int left, int right){
    if (left < right) {
        int mid = left + (right - left)/2;
        partition(nums, tmp, left, mid);
        partition(nums, tmp, mid+1, right);
        merge(nums, tmp, left, mid, right);
    }
}

template<typename T>
int quickpartition(std::vector<T> &arr, int left, int right){
    T p = arr[left];
    int i = left + 1;
    int j = right;
    while (true) {
        while (i <= j && arr[i] <= p) {
            ++i;
        }
        while (i <= j && arr[j] > p) {
            --j;
        }

        if (i > j) break;

        std::swap(arr[i], arr[j]);
    }
    std::swap(arr[j], arr[left]);
    return j;
}
template<typename T>
void quicksort(std::vector<T> &nums, int left, int right){
    if (left >= right) {
        return;
    }
    int mid = quickpartition(nums, left, right);

    quicksort(nums, left, mid);
    quicksort(nums,mid + 1, right);
}

// heap sort
//index 是 最大的非叶节点 下标;vsize 是 nums.size();
template<typename T>
void heapify(std::vector<T> &nums, int vsize, int index){
    int l_child = index * 2 + 1;
    int r_child = index * 2 + 2;
    int largest = index;
    if (l_child < vsize && nums[l_child] > nums[largest]) {
        largest = l_child;
    }
    if (r_child < vsize && nums[r_child] > nums[largest]) {
        largest = r_child;
    }
    if (index != largest) {
        std::swap(nums[index], nums[largest]);
        heapify(nums, vsize, largest);
    }
}

template<typename T>
void heapsort(std::vector<T> &nums){
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
    int arr_int[] = {61,17,29,22,34,60,73,56,89,12};
    int len = static_cast<int>(sizeof(arr_int)/sizeof(arr_int[1]));

    std::vector<int> arr = {61,17,29,22,34,60,73,56,89,12};
/*
    //bubble sort
    bubble_sort(arr_int, len);
*/
/*
    //select sort
    select_sort(arr_int, len);
*/
/*
    //insert sort
    insert_sort(arr_int, len);


    for (int i = 0;i < len;++i) {
        std::cout << arr_int[i] << " ";
    }
    std::cout << std::endl;
*/

/*
    //merge sort
    int n = arr.size();
    std::vector<int> tmp(n);
    partition(arr, tmp, 0, n-1);
*/
/*
    //quick sort
    int n = arr.size();
    quicksort(arr, 0, n-1);
*/
    //heap sort
    heapsort(arr);
    int n = arr.size();
    for (int i = 0;i < n;++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}