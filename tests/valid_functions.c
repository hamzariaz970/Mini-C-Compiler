int square(int n) {
    return n * n;
}

float average(int a, int b) {
    float total = a + b;
    return total / 2.0;
}

int main() {
    int x = square(5);
    float y = average(x, 7);
    if (y >= 10.0) {
        x = x + 1;
    }
    return x;
}
