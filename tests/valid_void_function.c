void touch(int value) {
    value = value + 1;
    return;
}

int main() {
    int x = 4;
    touch(x);
    return x;
}
