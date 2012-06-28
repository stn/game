//
//  claude.h
//  Chess program based on the Shannon's article
//
//  Shannon, C. E. (1950). Programming a computer for playing chess. Philosophical Magazine, 41(314), 256-275.
//  Taylor & Francis. Retrieved from http://portal.acm.org/citation.cfm?id=61701.67002
//
//  Created by Akira Ishino on 12/03/11.
//  Copyright (c) 2012 Akira Ishino. All rights reserved.

#ifndef game_claude_h
#define game_claude_h

#include <string>
#include <vector>
using namespace std;

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);               \
void operator=(const TypeName&)


class Move {
public:
  Move() {};
  Move(int from_x, int from_y, int to_x, int to_y, int piece);
  
  static Move Parse(string str);
  static Move Read();
  
  void SetFromTo(int from_x, int from_y, int to_x, int to_y);
  void SetFromToPiece(int from_x, int from_y, int to_x, int to_y, int piece);
  
  string ToString() const;
  void Print() const;
  
  int from_x() const { return from_x_; }
  int from_y() const { return from_y_; }
  int to_x() const { return to_x_; }
  int to_y() const { return to_y_; }
  int piece() const { return piece_; }
  
private:
  int from_x_;
  int from_y_;
  int to_x_;
  int to_y_;
  int piece_;
  
  friend bool operator==(const Move& lhs, const Move& rhs);
  friend bool operator!=(const Move& lhs, const Move& rhs);
};

inline bool operator==(const Move& lhs, const Move& rhs) {
  return lhs.from_x_ == rhs.from_x_ &&
  lhs.from_y_ == rhs.from_y_ &&
  lhs.to_x_ == rhs.to_x_ &&
  lhs.to_y_ == rhs.to_y_ &&
  lhs.piece_ == rhs.piece_;
}

inline bool operator!=(const Move& lhs, const Move& rhs){
  return !operator==(lhs,rhs);
}


class Position {
public:
  Position() {}
  
	void StartPosition();
	
  void Print() const;
  void DoMove(const Move&, Position* dst);
  bool IsCheck();
  void CalcMoves();
  bool IsValidMove(const Move&);
  
	int get_board(int x, int y) const { return board_[x][y]; }
	void set_board(int x, int y, int p) { board_[x][y] = p; }
  
  const vector<Move>& next_moves() const { return next_moves_; }
  
  string Fen() const;
  static void ParseFen(const string& fen, Position* pos);
  
private:
	void PartialCopyFrom(const Position& src);
  
  void CalcIsUnderAttack();
  void CalcIsUnderAttackByPawn(int, int);
  void CalcIsUnderAttackByKnight(int, int);
  void CalcIsUnderAttackByBishop(int, int);
  void CalcIsUnderAttackByRook(int, int);
  void CalcIsUnderAttackByQueen(int, int);
  void CalcIsUnderAttackByKing(int, int);
  int SetIsUnderAttack(int, int);
  
  void CalcPawnMoves(int, int);
  void CalcBishopMoves(int, int);
  void CalcKnightMoves(int, int);
  void CalcRookMoves(int, int);
  void CalcQueenMoves(int, int);
  void CalcKingMoves(int, int);
  void AddPawnMoves(int from_x, int from_y, int to_x, int to_y);
  int AddMove(int from_x, int from_y, int to_x, int to_y, int piece);
  
  int board_[8][8];
  int side_;
  
  // castling is available?
  bool can_castling_[2][2];
  
  // En passant target
  int en_passant_target_x_;
  int en_passant_target_y_;
  
  // Halfmove clock for 50 moves rule.
  int halfmove_clock_;
  
  // Fullmove counter.
  int fullmove_counter_;
  
  // The following part is not a part of Position?
  
  // next moves
  vector<Move> next_moves_;
  
  // is under attack?
  bool is_under_attack_[8][8];
};


class RandomPlayer {
public:
  RandomPlayer() {}
  bool NextMove(Position& pos, Move* next_move);
  
private:
  DISALLOW_COPY_AND_ASSIGN(RandomPlayer);
};

class MinMaxPlayer {
public:
  MinMaxPlayer(int max_depth);
  bool NextMove(Position& pos, Move* next_move);
  
  int count;
  int last_score;
  
private:
  bool CalcNextMove(Position& pos, int depth, Move* next_move, int* score);
  bool CalcOpponentNextMove(Position& pos, int depth, Move* next_move, int* score);
  int CalcScore(Position& pos);
  
  int max_depth_;
  
  DISALLOW_COPY_AND_ASSIGN(MinMaxPlayer);
};

#endif  // game_claude_h