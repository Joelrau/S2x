#pragma once

#include "game/game.hpp"
#include "party_session.hpp"

#include <utils/info_string.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace party
{
	struct internal_connect_request
	{
		std::uint64_t attempt_id{};
		session::descriptor session{};
		std::string map_name{};
		std::string gametype{};
	};

	game::netadr_s& get_target();
	void connect_to(const game::netadr_s& target);
	// Resumes a held, validated s2x_infoResponse
	void continue_connect(const game::netadr_s& from, const utils::info_string& info,
		int max_clients, std::uint64_t attempt_id);
	std::string get_advertised_map_name();
	bool is_connection_attempt_current(std::uint64_t attempt_id);
	void cancel_pending_connection();
	void queue_connect(std::string address);
	void execute_internal_connect(const internal_connect_request& request);

	bool resolve_map_index(const std::string& map_name, int& map_index);
	bool validate_gametype(const std::string& gametype);
	bool set_match_rules_gametype(const std::string& gametype);
	void apply_map_settings(const std::string& map_name, const std::string& gametype, int map_index);
	std::string loaded_map_name();
	std::string loaded_gametype();
	bool server_running();

	int get_connected_client_count();
	int get_available_match_slots();
}
