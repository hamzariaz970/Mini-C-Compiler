int global_count;

int helper(int n) {
    int local = n;
    return local;
}

int main() {
    int x = helper(4);
    return x;
}
