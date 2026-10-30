#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace wmi
{
	namespace
	{
		decltype(&CoInitializeEx) co_initialize_ex_original{};

		HRESULT WINAPI co_initialize_ex_stub(LPVOID reserved, const DWORD co_init)
		{
			// The global WMI context (MP 0xAB08F0 / SP 0x7E9480, run as a static initializer)
			// connects to root\CIMV2 and root\wmi, which can stall startup on systems with a
			// broken WMI service. When this CoInitializeEx fails the context stays empty:
			// every query helper (MP 0xAB0D90 / SP 0x7E9920) returns E_FAIL for a null service,
			// and the destructor skips CoUninitialize.
			if (reinterpret_cast<size_t>(_ReturnAddress()) == game::select(0xAB0915, 0x7E94A5))
			{
				return E_FAIL;
			}

			return co_initialize_ex_original(reserved, co_init);
		}
	}

	struct component final : generic_component
	{
	public:
		void post_load() override
		{
			// Must be in place before the game's CRT initializers run.
			auto* const entry = utils::nt::library{}.get_iat_entry("ole32.dll", "CoInitializeEx");
			if (!entry || !*entry)
			{
				return;
			}

			co_initialize_ex_original = static_cast<decltype(co_initialize_ex_original)>(*entry);
			utils::hook::set(entry, co_initialize_ex_stub);
		}
	};
}

REGISTER_COMPONENT(wmi::component)
