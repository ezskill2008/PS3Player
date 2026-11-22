#include <PS3Player/Emu/PPUFetch.h>

static inline uint64_t
sext(int64_t v) {
	return static_cast<uint64_t>(v);
}

void
fetchOperand(PPU& ppu, uint32_t index) {
	if (index >= 5) return;

	const InstOperand op{ ppu.instData.operands[index] };
	const InstField& f{ ppu.inst };
	uint64_t& out{ ppu.operands[index] };

	switch (op) {
	case InstOperand::RD:   out = f.rd.Get(); break;
	case InstOperand::RS:   out = f.rs.Get(); break;
	case InstOperand::RA:   out = f.ra.Get(); break;
	case InstOperand::RA0:  out = f.ra.Get(); break;
	case InstOperand::RB:   out = f.rb.Get(); break;

		// ---- FPR: destination gives the number, sources give the raw 64-bit pattern
	case InstOperand::FRD:  out = f.frd.Get(); break;
	case InstOperand::FRS:  out = f.frs.Get(); break;
	case InstOperand::FRA:  out = f.fra.Get(); break;
	case InstOperand::FRB:  out = f.frb.Get(); break;
	case InstOperand::FRC:  out = f.frc.Get(); break;

		// ---- VR: 128 bits don't fit in ops[], so always the register number
	case InstOperand::VD:   out = f.vd.Get(); break;
	case InstOperand::VS:   out = f.vs.Get(); break;
	case InstOperand::VA:   out = f.va.Get(); break;
	case InstOperand::VB:   out = f.vb.Get(); break;
	case InstOperand::VC:   out = f.vc.Get(); break;

		// ---- Immediates (signed ones are sign-extended to 64 bits)
	case InstOperand::SIMM:  out = sext(f.simm.Get()); break;
	case InstOperand::UIMM:  out = f.uimm.Get(); break;
	case InstOperand::DS:    out = sext(f.DispDS()); break;
	case InstOperand::VSIMM: out = sext(f.vsimm.Get()); break;
	case InstOperand::VUIMM: out = f.vuimm.Get(); break;
	case InstOperand::VSH:   out = f.vsh.Get(); break;
	case InstOperand::NB:    out = f.nb.Get(); break;
	case InstOperand::SH:    out = f.sh.Get(); break;
	case InstOperand::SH64:  out = f.Sh64(); break;
	case InstOperand::MB:    out = f.mb.Get(); break;
	case InstOperand::ME:    out = f.me.Get(); break;
	case InstOperand::MB64:  out = f.Mb64(); break;
	case InstOperand::ME64:  out = f.Me64(); break;

		// ---- Branch / CR (displacements are byte offsets, handler applies AA/pc)
	case InstOperand::BO:    out = f.bo.Get(); break;
	case InstOperand::BI:    out = f.bi.Get(); break;
	case InstOperand::BH:    out = f.bh.Get(); break;
	case InstOperand::BD:    out = sext(f.DispBD()); break;
	case InstOperand::LI:    out = sext(f.DispLI()); break;
	case InstOperand::CRFD:  out = f.crfd.Get(); break;
	case InstOperand::CRFS:  out = f.crfs.Get(); break;
	case InstOperand::CRBD:  out = f.crbd.Get(); break;
	case InstOperand::CRBA:  out = f.crba.Get(); break;
	case InstOperand::CRBB:  out = f.crbb.Get(); break;
	case InstOperand::CRM:   out = f.crm.Get(); break;
	case InstOperand::FM:    out = f.fm.Get(); break;

		// ---- Misc
	case InstOperand::L:     out = f.l.Get(); break;
	case InstOperand::TO:    out = f.to.Get(); break;
	case InstOperand::SPR:   out = f.Spr(); break;
	case InstOperand::SR:    out = f.sr.Get(); break;
	case InstOperand::LEV:   out = f.lev.Get(); break;

	default:                 out = 0; break;
	}
}

void
PPUFetchOperands(PPU& ppu) {
	for (uint32_t i{ 0 }; i < 5 && ppu.instData.operands[i] != InstOperand::None; ++i)
		fetchOperand(ppu, i);
}
