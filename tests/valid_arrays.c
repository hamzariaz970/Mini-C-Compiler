int main() {
    int values[4];
    int i = 0;
    int total = 0;

    values[0] = 3;
    values[1] = 4;
    values[2] = 5;
    values[3] = 6;

    while (i < 4) {
        total = total + values[i];
        i = i + 1;
    }

    return total;
}
