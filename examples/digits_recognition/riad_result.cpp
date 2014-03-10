#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <iomanip>
#include <tuple>
#include <memory>

#include "faml/data/table.hpp"
#include "faml/kernels.hpp"
#include "faml/distances.hpp"
#include "faml/io.hpp"
#include "faml/models/knn.hpp"
#include "faml/preprocessing/scaler.hpp"
#include "faml/cross_validation.hpp"
#include "faml/quality/classification.hpp"

using namespace std;
using namespace faml;
using namespace Eigen;

int main() {
	auto testData = readCSV("mnist_small_train.csv");
	auto testStr = readCSV("mnist_test_no_label.csv");
	typedef std::vector<std::string> StrRowType;
	typedef VectorXf SampleType;
	typedef unsigned long long Label;
	Table<StrRowType> trainXstr, trainYstr;
	std::tie(trainXstr, trainYstr) = testData.splitOnColumns({"label"});
	auto trainY = trainYstr.cast(
		[](const StrRowType &sample) { 
			return std::stoull(sample[0]); 
		}
	);

	auto trainX = trainXstr.castByElement<VectorXf>(
		[](const std::string& x) {
			return std::stod(x);
		}
	);

	auto test = testStr.castByElement<VectorXf>(
		[](const std::string& x) {
			return std::stod(x);
		}
	);
	trainXstr.clear();
	trainYstr.clear();

	std::vector<std::unique_ptr<Scaler<VectorXf>>> scalers;
	scalers.emplace_back(new DummyScaler<VectorXf>());
//	scalers.emplace_back(new PowerAmplifyScaler(1.4));
	//scalers.emplace_back(new NormalScaler());
	//scalers.emplace_back(new MinMaxScaler(columns.size(), 0, 1));

	std::vector<std::unique_ptr<KernelFunction>> kernels;
	kernels.emplace_back(new QuarticKernel());
	//	kernels.emplace_back(new DiscreteKernel());
	// kernels.emplace_back(new InverseKernel());
	//kernels.emplace_back(new RBFKernel(1.0));
	//kernels.emplace_back(new EpanechnikovKernel());

	std::vector<std::unique_ptr<DistanceFunction<VectorXf>>> distances;
//	distances.emplace_back(new EuclidianDistance());
//	distances.emplace_back(new MinkowskiDistance(3));
//	distances.emplace_back(new MinkowskiDistance(5));
	distances.emplace_back(new CosineDistance());
//	distances.emplace_back(new OverlapDistance());

	for(const auto& scaler: scalers) {
		scaler->train(trainX);
		auto lambda = [&scaler](const VectorXf& row) {
			return (*scaler)(row);
		};
		auto scaledX = trainX.cast(lambda);
		auto scaledTest = test.cast(lambda);
		for(size_t k = 19; k <= 19; ++k) {
			for(const auto& distance: distances) {
				for(const auto& kernel: kernels) {
					clock_t start = clock();
					KNNClassifier<VectorXf, Label> knn(k, *distance, *kernel);
					knn.train(scaledX, trainY);
					auto prediction = knn.predict(scaledTest);
					cout << "Id,Prediction" << "\n";
					for(size_t i = 0; i < prediction.rowsNumber(); ++i) {
						cout << i + 1 << "," << prediction[i] << "\n";
					}
//					cout << k << ' ' << scaler->toString() << ' ' << distance->toString() << ' ' << kernel->toString() << "\n";
//				cout << "Score: " << accuracyScore(subtestY, prediction) << "\n";
//					cout << "time " << (clock() - start) / 1.0 / CLOCKS_PER_SEC;
//					cout << endl;
				}
			}
		}
	}

	return 0;
}
