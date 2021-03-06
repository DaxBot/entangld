#include <cassert>
#include <ctime>

#include "Datastore.h"

using namespace entangld;

volatile bool test_done[2] = {false, false};

int main(int argc, char *argv[])
{
    time_t start;
    Datastore *store_a = new Datastore({
        {"name", "Alfred"},
        {"occupation", "Butler"}
    });

    Datastore *store_b = new Datastore({
        {"name", "Bruce"},
        {"occupation", "Batman"},
    });

    store_a->attach(
        "store_b",
        [](const Message &msg, void *ctx) {
            Datastore *store_b = static_cast<Datastore*>(ctx);
            store_b->receive(msg, "store_a");
        },
        store_b
    );

    store_b->attach(
        "store_a",
        [](const Message &msg, void *ctx) {
            Datastore *store_a = static_cast<Datastore*>(ctx);
            store_a->receive(msg, "store_b");
        },
        store_a
    );

    store_a->subscribe("store_b.name", [](const Message &msg, void*){
        assert(msg.value.at("first") == "Bruce");
        test_done[0] = true;
    });

    store_b->set("name", {
        {"first", "Bruce"},
        {"middle", nullptr},
        {"last", "Wayne"}
    });

    store_b->subscribe("store_a.name", [](const Message &msg, void*){
        assert(msg.value.at("first") == "Alfred");
        test_done[1] = true;
    });

    store_a->set("name", {
        {"first", "Alfred"},
        {"middle", "Thaddeus Crane"},
        {"last", "Pennyworth"}
    });

    time(&start);
    while(!test_done[0] || !test_done[1]) {
        if(difftime(time(nullptr), start) >= 1)
            return EXIT_FAILURE;
    }

    test_done[0] = false;
    test_done[1] = false;

    assert(store_b->unsubscribe("store_a.name") == 1);

    store_a->set("name.first", "Greg");
    store_b->set("name.first", "Bruce");

    time(&start);
    while(!test_done[0]) {
        if(difftime(time(nullptr), start) >= 1)
            return EXIT_FAILURE;
    }

    assert(test_done[1] == false);

    delete store_a;
    delete store_b;

    return EXIT_SUCCESS;
}
