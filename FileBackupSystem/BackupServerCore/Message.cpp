#include "pch.h"
#include "Message.h"
#include "BufferUtils.h"
#include <stdexcept>

#define MIN_HEADER_SIZE 6 // UserID(4) + Ver(1) + Op(1)

ParsedRequest Message::parseRequest(const std::vector<std::byte>& buffer) {
	ParsedRequest req;
	size_t offset = 0;
	validateHeaderSize(buffer);
	readHeader(buffer, offset, req); //read userId, version, op
	if(requestHasFilename(req.op)) {
		readFilename(buffer, offset, req);
	}
	if(requestHasPayload(req.op)) {
		readPayload(buffer, offset, req);
	}
	return req;

}

std::vector<std::byte> Message::packResponse(uint8_t version, uint16_t status, const std::string& filename, const std::vector<std::byte>& payload) {
	std::vector<std::byte> buffer;
	packHeader(buffer, version, status);
	if (responseHasFilename(status)) {
		packFilename(buffer, filename);
	}
	if (responseHasPayload(status)) {
		packPayload(buffer, payload);
	}
	return buffer;
}

bool Message::requestHasFilename(uint8_t op) {
	return op != OP_LIST_FILES;
}

bool Message::requestHasPayload(uint8_t op) {
	return op == OP_SAVE_FILE;
}
bool Message::responseHasFilename(uint16_t status) {
	return (status != STATUS_ERROR_NO_FILES && status != STATUS_ERROR_GENERAL);
}

bool Message::responseHasPayload(uint16_t status) {
	return (status == STATUS_SUCCESS_FILE_FOUND || status == STATUS_SUCCESS_LIST);
}

void Message::validateHeaderSize(const std::vector<std::byte>& buffer) {
	if (buffer.size() < MIN_HEADER_SIZE) {
		throw std::runtime_error("Invalid request: Buffer too short for header");
	}
}

void Message::readHeader(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req) {
	req.userId = BufferUtils::extractInt<uint32_t>(buffer, offset);
	req.version = static_cast<uint8_t>(buffer[offset++]);
	req.op = static_cast<uint8_t>(buffer[offset++]);
}

void Message::readFilename(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req) {
	req.filenameLen = BufferUtils::extractInt<uint16_t>(buffer, offset);
	if(offset+ req.filenameLen > buffer.size()) {
		throw std::runtime_error("Invalid request: Buffer too short for filename");
	}
	req.filename.clear();
	for (size_t i = 0; i < req.filenameLen; ++i) {
		req.filename += static_cast<char>(buffer[offset++]);
	}
}

void Message::readPayload(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req) {
	uint32_t payloadSize = BufferUtils::extractInt<uint32_t>(buffer, offset);
	if (offset + payloadSize > buffer.size()) {
		throw std::runtime_error("Invalid request: Buffer too short for payload");
	}
	req.payload.clear();
	if (payloadSize > 0) {
		auto startIt = buffer.begin() + offset;
		auto endIt = startIt + payloadSize;
		req.payload.assign(startIt, endIt);
		offset += payloadSize;
	}
}
void Message::packHeader(std::vector<std::byte>& buffer, uint8_t version, uint16_t status) {
	buffer.push_back(static_cast<std::byte>(version));
	BufferUtils::pushInt<uint16_t>(buffer, status);
}

void Message::packFilename(std::vector<std::byte>& buffer, const std::string& filename) {
	uint16_t nameLen = static_cast<uint16_t>(filename.length());
	BufferUtils::pushInt<uint16_t>(buffer, nameLen);
	BufferUtils::pushString(buffer, filename);
}

void Message::packPayload(std::vector<std::byte>& buffer, const std::vector<std::byte>& payload) {
	uint32_t dataSize = static_cast<uint32_t>(payload.size());
	BufferUtils::pushInt<uint32_t>(buffer, dataSize);
	buffer.insert(buffer.end(), payload.begin(), payload.end());
}