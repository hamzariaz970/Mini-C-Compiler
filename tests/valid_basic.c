int main() {
    int a = 10;
    int b = 2 + 3 * 4;
    float ratio = a / 2.0;
    bool ok = ratio > 4.5 && a != 0;

    if (ok) {
        a = a + b;
    } else {
        a = a - 1;
    }

    return a;
}
