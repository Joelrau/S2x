#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/lookup/errors.hpp"

#include <utils/hook.hpp>

namespace error_messages
{
	namespace
	{
		// Ship builds replace error formats with "<id> <specifiers>"; restore the original text
		utils::hook::detour com_error_hook;
		utils::hook::detour bg_anim_parse_error_hook;
		utils::hook::detour scr_object_error_hook;

		void* com_error_original{};
		void* bg_anim_parse_error_original{};
		void* scr_object_error_original{};

		const char* translate_format_stub(const char* fmt)
		{
			return fmt ? game::lookup::errors::translate_format(fmt).data() : fmt;
		}

		// Swap only the format argument and tail-jump to the original, so the variadic
		// arguments are forwarded untouched and the engine still does the formatting.
		void* create_format_stub(const Gp& format_register, void* const* original)
		{
			return utils::hook::assemble([format_register, original](utils::hook::assembler& a)
			{
				a.push(rcx);
				a.push(rdx);
				a.push(r8);
				a.push(r9);
				a.sub(rsp, 0x28);

				a.mov(rcx, format_register);
				a.call(translate_format_stub);

				a.add(rsp, 0x28);
				a.pop(r9);
				a.pop(r8);
				a.pop(rdx);
				a.pop(rcx);

				a.mov(format_register, rax);
				a.mov(r11, reinterpret_cast<size_t>(original));
				a.jmp(qword_ptr(r11));
			});
		}
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			com_error_hook.create(game::Com_Error, create_format_stub(rdx, &com_error_original));
			com_error_original = com_error_hook.get_original();

			// Scr_Error and Scr_Error2 are translated by the gsc script_extension hooks
			scr_object_error_hook.create(game::Scr_ObjectError, create_format_stub(rcx, &scr_object_error_original));
			scr_object_error_original = scr_object_error_hook.get_original();

			if (game::environment::uses_multiplayer_binary())
			{
				bg_anim_parse_error_hook.create(game::BG_AnimParseError,
					create_format_stub(rcx, &bg_anim_parse_error_original));
				bg_anim_parse_error_original = bg_anim_parse_error_hook.get_original();
			}
		}
	};
}

REGISTER_COMPONENT(error_messages::component)
