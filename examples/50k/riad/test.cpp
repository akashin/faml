#include "faml/io.hpp"
#include "faml/utility/utility.hpp"
#include "faml/models/tree.hpp"
#include "faml/models/tree/trainers/ID3PruningTrainer.hpp"
#include "faml/statistics/informativity_criteria.hpp"
#include "faml/cross_validation/cross_validation.hpp"
#include "faml/cross_validation/shuffle_split.hpp"
#include "faml/quality/classification.hpp"
#include "faml/preprocessing/scaler.hpp"
#include "faml/data/merge.hpp"
#include <vector>
#include <limits>

#include <iostream>
using namespace std;
using namespace faml;

template <typename Row>
class Discretizator: Scaler<Row> {
public:
	virtual ~Discretizator() {}
	Discretizator(size_t column, size_t C): column(column), C(C) {
	}
	virtual void train(const TableView<Row>& x) {
		borders.clear();
		vector<double> values;
		values.reserve(x.rowsNumber());
		for(const auto& row: x) {
			assert(row[column] != "?");
			values.push_back(std::stod(row[column]));
		}
		sort(values.begin(), values.end());
		for(int i = 1; i < C; ++i) {
			size_t a = i * values.size() / C;
			if(a + 1 < values.size())
				borders.push_back((values[a] + values[a + 1]) / 2);
		}
		for(int i = 1; i < C; ++i) {
			borders.push_back(values.front() + (values.back() - values.front()) * i / C);
		}
		sort(borders.begin(), borders.end());
		borders.erase(unique(borders.begin(), borders.end(), Equality(borders)), borders.end());
	}
	struct Equality {
		Equality(const vector<double>& borders): borders(borders) {}
		bool operator() (double x, double y) {
			return lower_bound(borders.begin(), borders.end(), x) == lower_bound(borders.begin(), borders.end(), y);
		}
	private:
		const vector<double>& borders;
	};

	virtual Row operator() (const Row& row) const {
		/*if(row[column] == "?") {
			return Row(borders.size() + 1, "?");
		}
		Row result(borders.size() + 1, "0");
		double value = std::stod(row[column]);
		for(size_t i = 0; i < borders.size(); ++i) {
			result[lower_bound(borders.begin(), borders.end(), value) - borders.begin()] = "1";
		}
		return result;
		*/
		if(row[column] == "?") {
			assert(false);
		}
		Row result(borders.size(), "0");
		double value = std::stod(row[column]);
		for(size_t i = 0; i < borders.size(); ++i) {
			if(value < borders[i])
				result[i] = "1";
			else
				break;
		}
		return result;
	}

	virtual Table<Row> operator() (const TableView<Row>& data) const {
		vector<string> features;
		string name = data.columnsNames()[column];
		double prev = -numeric_limits<double>::infinity();
		for(double border: borders) {
			//features.push_back(to_string(prev) + " < " + name + " < " + to_string(border));
			features.push_back(name + " < " + to_string(border));
			prev = border;
		}
//		features.push_back(to_string(prev) + " < " + name);
		return data.cast(
				[&](const Row& row) { return (*this)(row); },
				features
		);
	}

	virtual std::string toString() const {
		return "MyDiscretizator";
	}
private:
	size_t C;
	size_t column;
	vector<double> borders;
};
int main(int argc, char** argv) {
	if(argc < 2) {
		cerr << "usage: " << argv[0] << " filename";
		exit(1);
	}

	string file = argv[1];
	auto data = readCSV(file);
	Table<vector<string>> x, _y;
	std::tie(x, _y) = data.splitOnColumns({"50k"});
	auto y = _y.cast(firstElement<vector<string>>);
	typedef vector<string> Row;
	typedef string Label;
	std::vector<std::shared_ptr<Discretizator<Row>>> discs;
	vector<size_t> columns = {0, 2, 4, 10, 11, 12};
	for(auto column: columns) {
		discs.push_back(make_shared<Discretizator<Row>>(column, 4));
	}
	for(const auto& disc: discs) {
		disc->train(x);
		x = faml::merge(x, (*disc)(x));
	}
	cout << x.columnsNames().size() << ' ' << x[0].size() << endl;
	for(string s: x.columnsNames()) {
		cout << s << "\n";
	}
	auto predictor = std::make_shared<TreeClassifier<ID3PruningTrainer<Row, Label>>>(ID3PruningTrainer<Row, Label>(std::make_shared<EntropyCriteria<Label>>(), 0.7, 42));

	std::cout << crossValidate<Row, Label>(predictor, x, y, ShuffleSplit(x.rowsNumber(), 0.5000, 10), AccuracyScorer<Label>());


	return 0;
}
