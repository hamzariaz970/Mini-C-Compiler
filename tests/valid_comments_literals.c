#define IGNORED_BY_LEXER 1

int main() {
    // Single-line comments are skipped.
    int x = 1;
    char c = 'z';
    bool ok = true;
    /*
       Block comments are skipped too.
    */
    if (ok && c != 'a') {
        x = x + 2;
    }
    return x;
}
