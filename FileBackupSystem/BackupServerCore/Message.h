#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>


enum MessageOpcode : uint8_t {
	OP_SAVE_FILE = 200,
	OP_RESTORE_FILE = 201,
	OP_DELETE_FILE = 202,
	OP_LIST_FILES = 203
};

enum MessageStatus :uint16_t {
	STATUS_SUCCESS_FILE_FOUND=210,
	STATUS_SUCCESS_LIST = 211,
	STATUS_SUCCESS_GENERIC=212,
	STATUS_ERROR_FILE_NOT_FOUND = 1001,
	STATUS_ERROR_NO_FILES = 1002,
	STATUS_ERROR_GENERAL = 1003
};

struct ParsedRequest {
	uint32_t userId;
	uint8_t version;
	uint8_t op;
	uint16_t filenameLen;
	std::string filename;
	std::vector<std::byte> payload;

	ParsedRequest() :userId(0), version(0), op(0), filenameLen(0) {}
};

class Message {
public:
	/**
		 * Parses a raw byte buffer into a structured Request object.
		 * @param buffer The full raw data received from the network.
		 * @return A parsed request structure.
		 * @throws std::runtime_error if the buffer is too short or invalid.
		 */
	static ParsedRequest parseRequest(const std::vector<std::byte>& buffer);
	
	/**
	 * Packs data into a raw byte buffer ready to be sent as a Response.
	 * @param version Server version.
	 * @param status Response status code.
	 * @param filename (Optional) Filename to include in header.
	 * @param payload (Optional) Binary content to send.
	 * @return A vector of bytes representing the binary protocol message.
	 */
	static std::vector<std::byte> packResponse(uint8_t version, uint16_t status, const std::string& filename = "", const std::vector<std::byte>& payload = {});
private:
	static bool requestHasFilename(uint8_t op);
	static bool requestHasPayload(uint8_t op);
	static bool responseHasFilename(uint16_t status);
	static bool responseHasPayload(uint16_t status);
	static void validateHeaderSize(const std::vector<std::byte>& buffer);
	static void readHeader(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req);
	static void readFilename(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req);
	static void readPayload(const std::vector<std::byte>& buffer, size_t& offset, ParsedRequest& req);
	static void packHeader(std::vector<std::byte>& buffer, uint8_t verstion, uint16_t status);
	static void packFilename(std::vector<std::byte>& buffer, const std::string& filename);
	static void packPayload(std::vector<std::byte>& buffer, const std::vector<std::byte>& payload);
};