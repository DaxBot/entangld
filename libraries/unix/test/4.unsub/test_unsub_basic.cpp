#include <cassert>
#include <ctime>

#include "Datastore.h"

using namespace entangld;

volatile bool test_done[3] = {false, false, false};

int main(int argc, char *argv[])
{
    time_t start;
    Datastore *store = new Datastore;

    store->subscribe("string", [](const Message &msg, void*){
        assert(msg.value == "this is a string!");
        test_done[0] = true;
    });

    store->subscribe("number.int", [](const Message &msg, void*){
        assert(msg.value == 3);
        test_done[1] = true;
    });

    store->subscribe("number.double", [](const Message &msg, void*){
        assert(msg.value == 6.0221409e+23);
        test_done[2] = true;
    });

    store->set("string", "this is a string!");
    store->set("number.int", 3);
    store->set("number.double", 6.0221409e+23);

    time(&start);
    while(!test_done[0] || !test_done[1] || !test_done[2]) {
        if(difftime(time(nullptr), start) >= 1)
            return EXIT_FAILURE;
    }

    test_done[0] = false;
    test_done[1] = false;
    test_done[2] = false;

    assert(store->unsubscribe("number.int") == 1);

    store->set("string", "this is a string!");
    store->set("number.int", 42);
    store->set("number.double", 6.0221409e+23);

    time(&start);
    while(!test_done[0] || !test_done[2]) {
        if(difftime(time(nullptr), start) >= 1)
            return EXIT_FAILURE;
    }

    assert(test_done[1] == false);

    delete store;

    return EXIT_SUCCESS;
}
