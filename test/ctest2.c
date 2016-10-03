
int sum(int x)
{
    int i, sum;

    i   = 0;
    sum = 0;

    while (i <= x) {
        sum = sum + i;
    }

    return sum;
}

int main()
{
    int y; 
    y = sum(100);
    return y;
}
