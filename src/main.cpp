#include "server.h"
#include "utils.h"
int main() {
    Server server(8021);
    server.Start();
}