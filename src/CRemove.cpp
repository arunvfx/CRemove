//CRemove.Cpp
// Remove|Keep channels from the input based on wildcard entries.

static const char* const CLASS = "CRemove";
static const char* const HELP = "Remove channels from the image";


#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"
#include <vector>

using namespace DD::Image;
static const char* const operations[] = { "remove", "keep", 0 };


class CRemove : public NoIop {

	ChannelSet channels;
	std::string wildcard_entry;
	int operationActive; // 0 = remove, 1 = keep
	std::vector<std::string> wildcardChannels; // store channels temporiorly for multiple wildcard entries.
	const std::string delimiter = ",";

public:

	// default constructor
	CRemove(Node* node) : NoIop(node) {
		operationActive = 0;
		this->wildcard_entry = "";
	}

	// virtual destructor
	virtual ~CRemove() {
		for (int i = 0; i < wildcardChannels.size(); i++)
		{
			delete &wildcardChannels[i];
		}
	}

	// This function dose remove/keep channels as per wildcards.
	void _validate(bool);

	// define knobs for this Node.
	virtual void knobs(Knob_Callback) override;

	// returns class name.
	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }
	static const Description description;

};

/*
isWildcardMatch -> check wildcard pattern with the channelname and if it matchs, then returns true else false.
*/

static inline bool isWildcardMatch(std::string channelName, std::string pattern) {
	int channelNameIndex = 0, patternIndex = 0, lastWildcardIndex = -1, channelNameBacktrackIndex = -1, nextWildcardIndex = -1;

	while (channelNameIndex < channelName.size()) {
		if (patternIndex < pattern.size() && (pattern[patternIndex] == '?' || pattern[patternIndex] == channelName[channelNameIndex])) {
			// chars match
			++channelNameIndex;
			++patternIndex;
		}
		else if (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
			// wildcard, so chars match - store index.
			lastWildcardIndex = patternIndex;
			nextWildcardIndex = ++patternIndex;
			channelNameBacktrackIndex = channelNameIndex;

			//storing the pidx+1 as from there I want to match the remaining pattern 
		}
		else if (lastWildcardIndex == -1) {
			// no match, and no wildcard has been found.
			return false;
		}
		else {
			// backtrack - no match, but a previous wildcard was found.
			patternIndex = nextWildcardIndex;
			channelNameIndex = ++channelNameBacktrackIndex;
			/*backtrack string from previousbacktrackidx + 1 index to see if then new pidxand sidx have same chars,
			if that is the case that means wildcard can absorb the chars in b/w and still further we can run the algo, 
				if at later stage it fails we can backtrack*/
		}
	}
	for (int i = patternIndex; i < pattern.size(); i++) {
		if (pattern[i] != '*') return false;
	}
	return true;
	// true if every remaining char in p is wildcard
}




bool isChannelExists(std::vector<std::string> &vec, std::string& str) {
	// check channelName exists in  the vector and returns bool.

	auto it = std::find(vec.begin(), vec.end(), str);
	return it != vec.end();
}


void CRemove::_validate(bool for_real) {

	copy_info();

	// get all available channels.
	ChannelMask inputChannels = this->input0().info().channels();

	wildcard_entry += ", 0";	
	size_t position = 0;
	std::string token;


	// split wildcard entries and compute wildcard one by one against each channels.
	while ((position = wildcard_entry.find(delimiter)) != std::string::npos) {
		token = wildcard_entry.substr(0, position);
		token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());

		if (token != "" && (token[token.size() - 1] != '*') && (token.find(".") == std::string::npos)) {
			token.append(".*");
		}

		wildcard_entry.erase(0, position + delimiter.length());

		for (Channel channel_: inputChannels) {

			std::string channelName = getName(channel_);
			bool is_wildcard_match = isWildcardMatch(channelName, token);

			if ((is_wildcard_match && operationActive) || (!is_wildcard_match && !operationActive) && !isChannelExists(wildcardChannels, channelName)) {
				info_.turn_on(channel_);
				wildcardChannels.push_back(channelName);

			}

			else if ((is_wildcard_match && !operationActive) || (!is_wildcard_match && operationActive) && !isChannelExists(wildcardChannels, channelName))
				info_.turn_off(channel_);	
				wildcardChannels.push_back(channelName);
		}

	}

	wildcardChannels.clear();
	

	this->set_out_channels(channels); // Tells nuke what we changed.
}

void CRemove::knobs(Knob_Callback callback) {
	Enumeration_knob(callback, &operationActive, operations, "operation", "Operation");
	String_knob(callback, &wildcard_entry, "wildcard", "Wildcard");
	Tooltip(callback, "Wildcard entries (*) will be checked on each channel in the scene. \n"
		"To remove\keep particular channel in a channelSet, add .red to keep| remove the channel.");

}

// build CRemove node.
static Iop* build(Node* node) { return new CRemove(node); }
const Iop::Description CRemove::description(CLASS, "Channel/CRemove", build);