int main() {
    int i = 0;
    int sum = 0;

    while (i < 5) {
        sum = sum + i;
        i = i + 1;
    }

    for (int j = 0; j < 3; j = j + 1) {
        sum = sum + j;
    }

    return sum;
}
