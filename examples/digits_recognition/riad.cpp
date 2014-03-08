#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <iomanip>
#include <tuple>
#include <memory>

#include "faml/data.hpp"
#include "faml/kernels.hpp"
#include "faml/distances.hpp"
#include "faml/io.hpp"
#include "faml/models/knn.hpp"
#include "faml/preprocessing/scaler.hpp"

using namespace std;
using namespace faml;
using namespace Eigen;

const bool VERBOSE = false;
/*
int main(int argc, char **argv) {

	NormalScaler normalScaler;
	normalScaler.train(subtrainXTrain);

	for (auto &sample : subtrainXTrain) {
		sample = normalScaler(sample);
	}

	for (auto &sample : subtrainXTest) {
		sample = normalScaler(sample);
	}

	int power = 2;

	MinkowskiDistance<sampleType> minkowskiDistance(power);
    double gamma = 4.0;
	RBFKernel rbfKernel(gamma);
	InverseKernel inverseKernel(1.0);
    TriangleKernel triangleKernel;
    QuarticKernel quarticKernel;
    EpanechnikovKernel epanechnikovKernel;
    DiscreteKernel discreteKernel;

    // removeOutliers(&subtrainXTrain, &subtrainYTrain, &minkowskiDistance, 0.001);
    // removeOutliers(&subtrainX, &subtrainY, &minkowskiDistance, 0.001);

	MatrixXf weights = VectorXf::Ones(subtrainX.columnsNumber());
    // weights = findOptimalWeights(subtrainXTrain, subtrainYTrain, 0.1, power);
    // std::cerr << weights << std::endl;

	WMinkowskiDistance<sampleType> wminkowskiDistance(power, weights);
	// MulticlassWMinkowskiDistance multiclassWminkowskiDistance(power, multiclassWeights);
	// MahalanobisDistance mahalanobisDistance(inverseCovarianceMatrix);
	CosineDistance<sampleType> cosineDistance;
	OverlapDistance<sampleType> overlapDistance;

	prepareTimer.stop();

	{
		Timer knnRunsTimer("knn runs");
		for (size_t K = 1; K < 20; ++K) {
			KNNClassifier<sampleType, labelType> classifier(K, minkowskiDistance, quarticKernel);
			classifier.train(subtrainXTrain, subtrainYTrain);
			Table<labelType> subtrainYTestPrediction = classifier.predict(subtrainXTest);

			PredictionStatistics statistics = calculatePredictionStatistics(subtrainYTest, subtrainYTestPrediction);
			std::cout << "K: " << K << " Accuracy: " << statistics.accuracy << std::endl;
		}
	}
/*
	if (argc == 3) {
		std::string testsetFilename(argv[2]);
		Table<sampleType> testSamples(readCSV<sampleType>(testsetFilename));
		for (auto &sample : testSamples) {
			sample = normalScaler(sample);
		}

		KNNClassifier<sampleType, labelType> classifier(17, minkowskiDistance, rbfKernel);
		classifier.train(subtrainX, subtrainY);
		Table<labelType> subtrainYTestPrediction = classifier.predict(testSamples);
//		printPrediction("predictions.csv", subtrainYTestPrediction);
	}
*/

//i	return 0;
//}
std::random_device rd;
std::mt19937 gen(rd());

double uniformUnitRandom() {
	std::uniform_real_distribution<> uniformGenerator(0, 1);
	return uniformGenerator(gen);
}

template<typename DataType, typename LabelType>
void trainTestSplit(const Table<DataType> &samples, const Table<LabelType> &labels,
					Table<DataType> &trainSamples, Table<LabelType> &trainLabels,
					Table<DataType> &testSamples, Table<LabelType> &testLabels,
					double testProportion) {

	size_t N = samples.rowsNumber();

	for (size_t i = 0; i < N; ++i) {
		if (uniformUnitRandom() < testProportion) {
			testSamples.addRow(samples[i]);
			testLabels.addRow(labels[i]);
		} else {
			trainSamples.addRow(samples[i]);
			trainLabels.addRow(labels[i]);
		}
	}
}

int main() {
	auto testData = readCSV("mnist_small_train.csv");
	typedef std::vector<std::string> strRowType;
	typedef VectorXf SampleType;
	typedef unsigned long long Label;
	Table<strRowType> trainXstr, trainYstr;
	std::tie(trainXstr, trainYstr) = testData.splitOnColumns({"label"});
	auto trainY = trainYstr.cast(
		[](const strRowType &sample) { 
			return std::stoull(sample[0]); 
		}
	);

	auto trainX = trainXstr.castByElement<VectorXf>(
		[](const std::string& x) {
			return std::stod(x);
		}
	);

	Table<SampleType> subtrainX(trainX.getColumnsNames());
	Table<Label> subtrainY(trainY.getColumnsNames());
	Table<SampleType> subtestX(trainX.getColumnsNames());
	Table<Label> subtestY(trainY.getColumnsNames());

	trainTestSplit(trainX, trainY,
				subtrainX, subtrainY,
				subtestX, subtestY,
				0.05);
	std::vector<std::unique_ptr<Scaler<VectorXf>>> scalers;
	scalers.emplace_back(new DummyScaler<VectorXf>());
	scalers.emplace_back(new NormalScaler());
	scalers.emplace_back(new MinMaxScaler(28*28, 0, 1));

//	vector<unique_ptr<Scaler>> scalers = 
	return 0;
}
