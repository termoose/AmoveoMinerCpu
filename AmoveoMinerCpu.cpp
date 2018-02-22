#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <iomanip>
#include <string>
#include <cassert>

#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <iomanip>
#include <string>
#include <cassert>

#include <future>
#include <numeric>
#include <chrono>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/http_listener.h>              // HTTP server
#include <cpprest/json.h>                       // JSON library
#include <cpprest/uri.h>                        // URI library

#include "base64.h"
#include "sha256.h"


using namespace std;
using namespace std::chrono;

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace web::http::experimental::listener;          // HTTP server
using namespace web::json;                                  // JSON library


json::array GetWork(string minerPublicKeyBase64);
void SubmitWork(string nonceBase64, string minerPublicKeyBase64);

unsigned int hash2integer(vector<BYTE> h);
static unsigned int pair2sci(unsigned int l[2]);
static vector<BYTE> next_nonce(vector<BYTE> nonce);


int gElapsedSecMax = 5;

//#define POOL_URL "http://159.89.106.253:8085"	// main node, set USE_SHARE_POOL 0
//#define POOL_URL "http://localhost:32371/work"	// local pool
#define POOL_URL "http://amoveopool.com/work"
#define USE_SHARE_POOL 1

#define MINER_THREADS 4

//#define MINER_ADDRESS "BN7s4sq3L20MMrehffEXwJi9mwH5df5r3T/zPdc/vR1OkwIvcBbUQdmtlB41z9TTg/ldgr86yFpMRIxiAp0B1ZY="
//#define MINER_ADDRESS "BMblRgpUyYDgvF1trmqR02Vobat/JoQTTPCL91jUeGKSDDXaGbAeIKrI0ftR9FxXenXK89/+tWn76Qy9RJlzKwM="
//#define MINER_ADDRESS "BPjR3zjCwtiRWBe/sov0oPOPNWxK2yHIIs4DNNNV/gjlhIp95/2uu8I2JPMJsPrxDeLOidHBYdMwaY8WGPmffYM="
//#define MINER_ADDRESS "BFF/slyA0khrMdzi5LOy17fygyTbEWXtyVqhLe7FAgWgfuqjvAAIeYwjI7lDkpRnoOYvPKeIAiQ0h++6w8mhp5U="
//#define MINER_ADDRESS "BL1axAXtft4NWeZ3NJrAbhTrlI2rAqgLxTzKLAry2HIFoNgKEEXIDb84FyXEIT0J0OYpMghWqvkkv3brBSVLaAQ="
//#define MINER_ADDRESS "BCQUvXKpEiLwdwf2DUvshryPN8ejTMWl+4R8B1v+LLLOE2yvwnuQ+CMPNhpWGl5kqVgyMMbuqWkTfzIRckZLNK8="
#define MINER_ADDRESS "BPA3r0XDT1V8W4sB14YKyuu/PgC6ujjYooVVzq1q1s5b6CAKeu9oLfmxlplcPd+34kfZ1qx+Dwe3EeoPu0SpzcI="


string gMinerPublicKeyBase64(MINER_ADDRESS);
int gMinerThreads = MINER_THREADS;
string gPoolUrl(POOL_URL);
wstring gPoolUrlW;

class Metrics
{
	_int64 hashesTried;
	int blocksFound;
	std::mutex mutex;
public:
	Metrics() {
		hashesTried = 0;
		blocksFound = 0;
	}
	_int64 getHashesTried() { return hashesTried; }
	void addHashesTried(int hashCount)
	{
		mutex.lock();
		hashesTried += hashCount;
		mutex.unlock();
	}
};

#ifdef _WIN32
#include <windows.h>

void sleep(unsigned milliseconds)
{
	Sleep(milliseconds);
}
#else
#include <unistd.h>

void sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000); // takes microseconds
}
#endif

// Prints a 32 bytes sha256 to the hexadecimal form filled with zeroes
void print_hash(const unsigned char* sha256) {
	for (size_t i = 0; i < 32; ++i) {
		std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(sha256[i]);
	}
	std::cout << std::dec << std::endl;
}

unsigned int hash2integer(vector<BYTE> h) {
	unsigned int x = 0;
	unsigned int y[2];
	for (int i = 0; i < 31; i++) {
		if (h[i] == 0) {
			x += 8;
			y[1] = h[i + 1];
			continue;
		}
		else if (h[i] < 2) {
			x += 7;
			y[1] = (h[i] * 128) + (h[i + 1] / 2);
		}
		else if (h[i] < 4) {
			x += 6;
			y[1] = (h[i] * 64) + (h[i + 1] / 4);
		}
		else if (h[i] < 8) {
			x += 5;
			y[1] = (h[i] * 32) + (h[i + 1] / 8);
		}
		else if (h[i] < 16) {
			x += 4;
			y[1] = (h[i] * 16) + (h[i + 1] / 16);
		}
		else if (h[i] < 32) {
			x += 3;
			y[1] = (h[i] * 8) + (h[i + 1] / 32);
		}
		else if (h[i] < 64) {
			x += 2;
			y[1] = (h[i] * 4) + (h[i + 1] / 64);
		}
		else if (h[i] < 128) {
			x += 1;
			y[1] = (h[i] * 2) + (h[i + 1] / 128);
		}
		else {
			y[1] = h[i];
		}
		break;
	}
	y[0] = x;
	return(pair2sci(y));
}
static unsigned int pair2sci(unsigned int l[2]) {
	return((256 * l[0]) + l[1]);
}

int check_pow(vector<BYTE> nonce, int blockDifficulty, int shareDifficulty, vector<BYTE> data) {
	BYTE text[66];//32+2+32
	for (int i = 0; i < 32; i++)
		text[i] = data[i];
	text[32] = blockDifficulty / 256;
	text[33] = blockDifficulty % 256;
	for (int i = 0; i < 32; i++)
		text[i + 34] = nonce[i];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, text, 66);
	vector<BYTE> buf(32);
	sha256_final(&ctx, &buf[0]);
	int i = hash2integer(buf);
	return(i >= shareDifficulty);
}
static vector<BYTE> next_nonce(vector<BYTE> nonce) {
	//we should use 32 bit or 64 bit integer
	for (int i = 0; i < 32; i++) {
		if (nonce[i] == 255) {
			nonce[i] = 0;
		}
		else {
			nonce[i] += 1;
			return nonce;
		}
	}
	vector<BYTE> empty(32);
	return empty;
}

struct MinerThreadData
{
	string publicKeyBase64;
	vector<BYTE> bhash;
	//vector<BYTE> nonce;
	int blockDifficulty;
	int shareDifficulty;
};



static bool miner_thread(std::atomic_bool & run, MinerThreadData &data, vector<BYTE> threadNonce, Metrics &metrics)
{
	int hashesTried = 0;

	try {
		vector<BYTE> nonce(threadNonce);

		while (run == true) {
			hashesTried++;
			if (check_pow(nonce, data.blockDifficulty, data.shareDifficulty, data.bhash)) {
				//cout << "bhash: ";
				//print_hash(&data.bhash[0]);
				//cout << "nonce: ";
				//print_hash(&nonce[0]);
				cout << "FoundShare: bDiff:" << data.blockDifficulty << " sDiff:" << data.shareDifficulty << endl;
				break;
			}
			nonce = next_nonce(nonce);
			if (hashesTried > 100000) {
				metrics.addHashesTried(hashesTried);
				hashesTried = 0;
			}
		}

		if (run == true)
		{
			string resultNonce = base64_encode(&nonce[0], 32);
			SubmitWork(resultNonce, data.publicKeyBase64);
		}
	}
	catch (...) {
		cout << "Typeless exception in miner_thread." << endl;
	}

	metrics.addHashesTried(hashesTried);
	return true;
}

void GetThreadData(MinerThreadData * pData)
{
	json::array workDataArray = GetWork(gMinerPublicKeyBase64);

	if (USE_SHARE_POOL == 1) {
		wstring wBHhashBase64(workDataArray.at(1).as_string().c_str());
		string bhashBase64(wBHhashBase64.begin(), wBHhashBase64.end());
		string bhashString = base64_decode(bhashBase64);
		vector<BYTE> bhash(bhashString.begin(), bhashString.end());
		pData->bhash = bhash;

		int blockDifficulty = workDataArray.at(2).as_integer();
		pData->blockDifficulty = blockDifficulty;

		int shareDifficulty = workDataArray.at(3).as_integer();
		pData->shareDifficulty = shareDifficulty;
	} else {
		wstring wBHhashBase64(workDataArray.at(1).as_string().c_str());
		string bhashBase64(wBHhashBase64.begin(), wBHhashBase64.end());
		string bhashString = base64_decode(bhashBase64);
		vector<BYTE> bhash(bhashString.begin(), bhashString.end());
		pData->bhash = bhash;
		/* Server nonce isn't used, but here's how to consume it if ever needed
		wstring wNonceBase64(workDataArray.at(2).as_string().c_str());
		string nonceBase64(wBHhashBase64.begin(), wBHhashBase64.end());
		string nonceString = base64_decode(nonceBase64);
		vector<BYTE> nonce(nonceString.begin(), nonceString.end());
		pData->nonce = nonce;
		*/
		int difficulty = workDataArray.at(3).as_integer();
		pData->blockDifficulty = difficulty;
		pData->shareDifficulty = difficulty;
	}
	pData->publicKeyBase64 = gMinerPublicKeyBase64;

}


int main(int argc, char* argv[])
{
	if (argc <= 1) {
		cout << "Example Template: " << endl;
		cout << argv[0] << " " << "<Base64AmoveoAddress>" << " " << "<Threads>" << " " << "<PoolUrl>" << endl;

		cout << endl;
		cout << "Example Usage: " << endl;
		cout << argv[0] << " " << MINER_ADDRESS << " " << MINER_THREADS << " " << POOL_URL << endl;

		cout << endl;
		cout << endl;
		cout << "Threads and PoolUrl are optional. Default Threads is 4. Default PoolUrl is http://amoveopool.com/work" << endl;
		return -1;
	}
	if (argc >= 2) {
		gMinerPublicKeyBase64 = argv[1];
	}
	if (argc >= 3) {
		gMinerThreads = atoi(argv[2]);
	}
	if (argc >= 4) {
		gPoolUrl = argv[3];
	}

	gPoolUrlW.resize(gPoolUrl.length(), L' ');
	std::copy(gPoolUrl.begin(), gPoolUrl.end(), gPoolUrlW.begin());

	srand(time(NULL));

	// rand() gives very sequential (non-random) numbers, so hash it to get a more random result
	unsigned int id = rand();
	SHA256_CTX ctx;
	vector<BYTE> hashBuf(32);

	Metrics metrics;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	_int64 hashPerSecond = 0;
	_int64 sharesFoundTotal = 0;
	_int64 secPerHour = 60 * 60;

	MinerThreadData minerThreadData;
	GetThreadData(&minerThreadData);
	std::chrono::steady_clock::time_point workDataBegin = std::chrono::steady_clock::now();

	int elapsedSecThreshold = gElapsedSecMax;
	_int64 elapsedSec;

	while (true)
	{
		std::atomic_bool run = true;

		bool anyMinerSucceeded = false;

		vector<future<bool>> minerFutures;
		vector<vector<BYTE>> minerNonces;
		for (int idx = 0; idx < gMinerThreads; idx++) {
			// There must be a better way to generate random nonces for each thread to use.
			sha256_init(&ctx);
			sha256_update(&ctx, (BYTE*)&id, sizeof(id));
			sha256_final(&ctx, &hashBuf[0]);
			id = *((unsigned int*)&hashBuf[0]);

			vector<BYTE> threadNonce(hashBuf);

			minerFutures.push_back(std::async(std::launch::async, miner_thread, std::ref(run), std::ref(minerThreadData), threadNonce, std::ref(metrics)));
		}

		do
		{
			minerFutures[0].wait_for(milliseconds(1000));

			for (int idx = 0; idx < gMinerThreads; idx++) {
				if (minerFutures[idx].wait_for(milliseconds(0)) == future_status::ready) {
					anyMinerSucceeded = true;
					break;
				}
			}

			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - workDataBegin).count();

			long secondDuration = std::chrono::duration_cast<std::chrono::seconds>(now - begin).count();
			cout << "H/S: " << metrics.getHashesTried() / secondDuration << " S:" << sharesFoundTotal << " S/Hr:" << (sharesFoundTotal * secPerHour) / secondDuration << endl;

		} while (anyMinerSucceeded == false && elapsedSec < elapsedSecThreshold);

		if (anyMinerSucceeded == true) {
			sharesFoundTotal++;
		}

		MinerThreadData minerThreadDataNew;
		GetThreadData(&minerThreadDataNew);

		if (memcmp(&minerThreadDataNew.bhash[0], &minerThreadData.bhash[0], 32) == 0) {
			elapsedSecThreshold += 3; // When work doesn't change, add 3 seconds to timeout threshold
			minerThreadData.shareDifficulty = minerThreadDataNew.shareDifficulty; // Even when bhash doesn't change, shareDiff can be updated
		} else {
			cout << "### NEW WORK ###" << endl;
			minerThreadData.bhash = minerThreadDataNew.bhash;
			minerThreadData.blockDifficulty = minerThreadDataNew.blockDifficulty;
			minerThreadData.shareDifficulty = minerThreadDataNew.shareDifficulty;
			//minerThreadData.nonce = minerThreadDataNew.nonce;
			elapsedSecThreshold = gElapsedSecMax;
			workDataBegin = std::chrono::steady_clock::now();
		}

		run = false;
	}
	

	return 0;
}



static void SubmitWork(string nonceBase64, string minerPublicKeyBase64)
{
	try {
		http_client client(gPoolUrlW);
		http_request request(methods::POST);
		std::stringstream body;
		body << "[\"work\",\"" << nonceBase64 << "\",\"" << minerPublicKeyBase64 << "\"]";
		request.set_body(body.str());

		http_response response = client.request(request).get();
		if (response.status_code() == status_codes::OK)
		{
			// Response data comes in as application/octet-stream, so extract_json throws an exception
			// Need to use extract_vector and then convert to string and then to json
			std::vector<unsigned char> responseData = response.extract_vector().get();

			wstring responseString(responseData.begin(), responseData.end());
			wcout << responseString << endl;
		} else {
			wcout << "ERROR: SubmitWork: " << response.status_code() << endl;
		}
	} catch( ... ) {
		wcout << "ERROR: SubmitWork caught typeless exception." << endl;
	}
}

json::array GetWork(string minerPublicKeyBase64)
{
	bool success = false;
	do {
		try {
			http_client client(gPoolUrlW);
			http_request request(methods::POST);
			if (USE_SHARE_POOL == 1) {
				std::stringstream body;
				body << "[\"mining_data\",\"" << minerPublicKeyBase64 << "\"]";
				request.set_body(body.str());
			}
			else {
				request.set_body(L"[\"mining_data\"]");
			}

			http_response response = client.request(request).get();
			if (response.status_code() == status_codes::OK)
			{
				// Response data comes in as application/octet-stream, so extract_json throws an exception
				// Need to use extract_vector and then convert to string and then to json
				std::vector<unsigned char> responseData = response.extract_vector().get();

				wstring responseString(responseData.begin(), responseData.end());

				json::value jsonResponse = json::value::parse(responseString);
				json::array dataArray = jsonResponse.as_array();
				return dataArray[1].as_array();
				success = true;
			}
			else {
				wcout << "ERROR: GetWork: " << response.status_code() << " Sleep and retry..."<< endl;
				sleep(3000);
			}
		} catch( ... ) {
			wcout << "ERROR: GetWork failed. Sleep and retry..." << endl;
			sleep(3000);
		}
	} while(!success);
}
