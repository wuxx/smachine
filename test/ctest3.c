
int main()
{
    int x, y, z; 
    int s;
    
    x = 1;
    y = 2;
    z = 3;
    
    if ((x - 1) != 0) {
        return 100;
    }

    s = x + y + z; 

    if (s != 6) {
        return 1000;
    }

    s = x - y - z;

    if (s != -4) {
        return 2000;
    }

    s = x * y * z;

    if (s != 6) {
        return 3000;
    }

    s = (x / y) / z;
    if (s != 0) {
        return 4000;
    }

    s = (x * y) / z;

    if (s != 0) {
        return 5000;
    }

    s = (x + y) / z;

    if (s != 1) {
        return 6000;
    }

    if ((x - 1) != 0 || (y - 2) != 0 || (z - 3) != 0) {
        return 7000;
    }

    if (x == 1 && y == 2 && z == 3) {
        return 0;
    } else {
        return 10000;
    }

    return 20000;

}
