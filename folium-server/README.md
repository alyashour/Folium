# Folium Server

## To build
Needs c++20, cmake (min 3.10).

1. Create the build dir with `mkdir build`
2. `cd` into build
3. Call cmake with `cmake ..`
4. Call `make`

Then run the program using
`bin/folium-server` and run tests using `bin/tests`.

## To setup MySQL DB

1. Make sure you have MySQL installed.
2. Create a database for example named **"folium"**
3. Use that database
4. Run `source "direct/path/to/seed.sql"`
5. create a file named `dbConfig.json` on the root of folium-server and have the following content skeleton:

```json
{
    "mysql_host": "127.0.0.1",
    "mysql_port": 3306,
    "mysql_user": "root",
    "mysql_password": "",
    "mysql_database": "folium"
}
```
***This is an example, change it based off of your db configuration.***