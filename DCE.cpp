//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;

#define DEBUG_TYPE "Dce"

STATISTIC(DCEEliminatedCount, "Counts number of instruction removed");

bool isInstructionCritical(Instruction *I) {
  if(!I->use_empty() || isa<llvm::TerminatorInst>(I))
    return true;
  if(!I->mayHaveSideEffects())
    return false;
  return true;
}

namespace {
  // Dead Code Elimination (instruction level)
  struct DCE : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    DCE() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
		bool changeMade = false;
		//Mark
		std::vector<Instruction*> WorkList, MarkedList;
      for(Function::iterator BB = F.begin(), FE = F.end(); BB != FE; BB++) {
        for(BasicBlock::iterator DI = BB->begin(), BE = BB->end(); DI != BE; DI++) {
          Instruction *Inst = DI;
		  WorkList.push_back(&*Inst);
		  if(isInstructionCritical(Inst))
			  MarkedList.push_back(&*Inst);
		  /*
		  printf("%10s %5d %5d %5d", Inst->getOpcodeName(Inst->getOpcode()), Inst->use_empty(), isa<llvm::TerminatorInst>(Inst), Inst->mayHaveSideEffects());
		  printf("USED ");
		  for(User::op_iterator OI = Inst->op_begin(); OI != Inst->op_end(); OI++)
			  if(Instruction *Used = dyn_cast<Instruction>(*OI))
				  printf("%10s", Used->getOpcodeName(Used->getOpcode()));
		  printf("DEFINED ");
		  if(isa<Instruction>(Op->get())){
			  Instruction *Used = cast<Instruction>(Op->get());
			  printf("%10s", Used->getOpcodeName(Used->getOpcode()));
		  }
		  printf("\n");
		  */
        }
      }
	  //printf("pushdone\n");
	  while(!MarkedList.empty()){
		Instruction *I = MarkedList.back();
		MarkedList.pop_back();
		for(Instruction::op_iterator Op = I->op_begin(), IE = I->op_end(); Op != IE; Op++){
			if(isa<Instruction>(Op->get())) {
				Instruction *defI = cast<Instruction>(Op->get());
				MarkedList.push_back(&*defI);
			}
		}
		WorkList.erase(std::remove(WorkList.begin(), WorkList.end(), I), WorkList.end());
//		printf("%d\n", WorkList.size());
	  }
	  //printf("markdone\n");
	  //Sweep
	  for(std::vector<Instruction*>::iterator it=WorkList.begin(), WE=WorkList.end(); it != WE; it++){
		  //printf("%10s", (*it)->getOpcodeName((*it)->getOpcode()));
		  (*it)->eraseFromParent();
		  DCEEliminatedCount++;
		  changeMade = true;
	  }
      return changeMade;
    }
  };
}

char DCE::ID = 0;
static RegisterPass<DCE>
X("Dce", "Dead Code Elimination Pass");
