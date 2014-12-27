#include "json.hpp"

#include "json_data_store.hpp"

struct Tester {
    int _i;
    int _i2;
};

JSON_ENABLE(Tester, (_i), (_i2));

#ifndef JSON_TEST_ALL
int main() {
    tjson::detail::DataStore<Tester> _storage;

    _storage.data_list.data.write(1);
    tjson::detail::data_list_next(_storage.data_list).data.write(2);

    Tester test = _storage.realize();
    std::cout << tjson::to_json(test) << std::endl;
}
#endif
