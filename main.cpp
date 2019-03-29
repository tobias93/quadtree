#include <iostream>
#include "quadtree.hpp"
#include "cities.hpp"

using namespace std;
using namespace quadtree;


void printCityPoint(const Point<City>& p) {
	cout << p.data.name << endl;
	cout << "    Population: " << p.data.population << endl;
	cout << "    Coordinate: (" << p.position.x << ", " << p.position.y << ")" << endl;
}

int main() {

    auto cities = Tree<City>(Vec2f{-180, -90}, Vec2f{180, 90});

    // Insert cities
    
    for (auto entry: largeCities) 
    {
    	float x, y;
    	City city;
    	std::tie(x, y, city) = entry;
    	cities.insert(Vec2f{x, y}, city);
    }
    
    // Query some information

	cout << endl;
	cout << "Cities near the equator:" << endl;
	auto onEquator = cities.query(Vec2f{-180, -10}, Vec2f{180, 10});
	for (auto& cityPoint: onEquator)
	{
		printCityPoint(cityPoint);
	}

	cout << endl;
	cout << "Cities near the prime meridian:" << endl;
	auto onMeridian = cities.query(Vec2f{-10, -90}, Vec2f{10, 90});
	for (auto& cityPoint: onMeridian)
	{
		printCityPoint(cityPoint);
	}
	cout << endl;

    return 0;
}