//
//  claude.cc
//  Chess program based on the Shannon's article
//
//  Shannon, C. E. (1950). Programming a computer for playing chess. Philosophical Magazine, 41(314), 256-275.
//  Taylor & Francis. Retrieved from http://portal.acm.org/citation.cfm?id=61701.67002
//
//  Created by Akira Ishino on 12/03/11.
//  Copyright (c) 2012 Akira Ishino. All rights reserved.

#include "claude.h"

#include <algorithm>
#include <sstream>
#include <iostream>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {
  
  int a2piece(char a) {
    switch (a) {
      case 'P':
        return 1;
        
      case 'N':
        return 2;
        
      case 'B':
        return 3;
        
      case 'R':
        return 4;
        
      case 'Q':
        return 5;
        
      case 'K':
        return 6;
        
      case 'p':
        return -1;
        
      case 'n':
        return -2;
        
      case 'b':
        return -3;
        
      case 'r':
        return -4;
        
      case 'q':
        return -5;
        
      case 'k':
        return -6;
        
      default:
        // error!
        break;
    }
    return 0;
  }
  
  char piece2a(int p) {
    const static char* PIECE_MARK = "kqrbnp.PNBRQK";
    
    if (p >= -6 && p <= 6) {
      return PIECE_MARK[p + 6];
    }
    return '?';
  }
  
}  // namespace


// Move

/* This default constructor causes 50% slow down */
// Move::Move()
//    : from_x_(0),
//      from_y_(0),
//      to_x_(0),
//      to_y_(0),
//      piece_(0) {};

Move::Move(int from_x, int from_y, int to_x, int to_y, int piece)
: from_x_(from_x),
from_y_(from_y),
to_x_(to_x),
to_y_(to_y),
piece_(piece) {};

void
Move::SetFromTo(int from_x, int from_y, int to_x, int to_y) {
  from_x_ = from_x;
  from_y_ = from_y;
  to_x_ = to_x;
  to_y_ = to_y;
  piece_ = 0;
}

void Move::SetFromToPiece(int from_x, int from_y, int to_x, int to_y, int piece) {
  from_x_ = from_x;
  from_y_ = from_y;
  to_x_ = to_x;
  to_y_ = to_y;
  piece_ = piece;
}

string
Move::ToString() const {
  ostringstream oss;
  oss.put('a' + from_x_);
  oss << from_y_ + 1;
  oss.put('a' + to_x_);
  oss << to_y_ + 1;
  if (piece_ != 0) {
    oss.put(piece2a(piece_));
  }  
  return oss.str();
}

// TODO: Remove Print().
void
Move::Print() const {
  printf("%s", ToString().c_str());
}

/* static */
Move
Move::Parse(string str) {
  switch (str.size()) {
    case 4:
      return Move(str[0] - 'a', str[1] - '1', str[2] - 'a', str[3] - '1', 0);
      
    case 5:
      return Move(str[0] - 'a', str[1] - '1', str[2] - 'a', str[3] - '1', a2piece(str[4]));
      
    default:
      // error!
      cerr << "Invalid move string: " << str << endl;
      break;
  }
  return Move(-1, -1, -1, -1, 0);
}

/* static */
Move
Move::Read() {
  printf("? ");
  char buf[100];
  if (fgets(buf, 100, stdin)) {
    return Move::Parse(buf);
  }
  return Move(-1, -1, -1, -1, 0);
}


// Position

void Position::StartPosition() {
  const static int init_board_[] = {
    4, 2, 3, 5, 6, 3, 2, 4,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -4, -2, -3, -5, -6, -3, -2, -4
  };
  
  int i = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      board_[x][y] = init_board_[i];
      ++i;
    }
  }
  side_ = 1;
  can_castling_[0][0] = true;
  can_castling_[0][1] = true;
  can_castling_[1][0] = true;
  can_castling_[1][1] = true;
  en_passant_target_x_ = -1;
  //en_passant_target_y_ = -1; // do not need
  halfmove_clock_ = 0;
  fullmove_counter_ = 1;
  next_moves_.clear();
}

void Position::PartialCopyFrom(const Position& src) {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      board_[x][y] = src.board_[x][y];
    }
  }
  // Faster than the above but too optimized?
  // memcpy(board_, src.board_, sizeof(board_));
  side_ = src.side_;
  for (int i = 0; i < 2; ++i) {
    can_castling_[i][0] = src.can_castling_[i][0];
    can_castling_[i][1] = src.can_castling_[i][1];
  }
  //en_passant_target_x_ = src.en_passant_target_x_; // do not need?
  //en_passant_target_y_ = src.en_passant_target_y_;
  halfmove_clock_ = src.halfmove_clock_;
  fullmove_counter_ = src.fullmove_counter_;
  next_moves_.clear();
}

void Position::Print() const {
  for (int y = 7; y >= 0; --y) {
    for (int x = 0; x < 8; ++x) {
      printf("%c", piece2a(board_[x][y]));
    }
    printf("\n");
  }
  for (vector<Move>::const_iterator it = next_moves_.begin(); it != next_moves_.end(); ++it) {
    it->Print();
  }
  printf("\n\n");
}

string Position::Fen() const {
  const static char* PIECE_MARK = "kqrbnp.PNBRQK";
  
  ostringstream os;
  // board
  for (int y = 7; y >= 0; --y) {
    for (int x = 0; x < 8; ++x) {
      if (board_[x][y] == 0) {
        int count = 1;
        for (++x; x < 8; ++x) {
          if (board_[x][y] == 0) {
            ++count;
          } else {
            --x;
            break;
          }
        }
        os << count;
      } else if (board_[x][y] >= -6 && board_[x][y] <= 6) {
        os << PIECE_MARK[board_[x][y] + 6];
      } else {
        os << '?';
      }
    }
    if (y > 0) {
      os << '/';
    }
  }
  os << ' ';
  //  side to move
  switch (side_) {
    case -1:
      os << "b ";
      break;
      
    case 1:
      os << "w ";
      break;
      
    default:
      os << "? ";
  }
  // castling ability
  bool has_castling_ability = false;
  if (can_castling_[0][1]) {
    os << 'K';
    has_castling_ability = true;
  }
  if (can_castling_[0][0]) {
    os << 'Q';
    has_castling_ability = true;
  }
  if (can_castling_[1][1]) {
    os << 'k';
    has_castling_ability = true;
  }
  if (can_castling_[1][0]) {
    os << 'q';
    has_castling_ability = true;
  }
  if (!has_castling_ability) {
    os << '-';
  }
  os << ' ';
  
  // en passant target
  if (en_passant_target_x_ == -1) {
    os << "- ";
  } else {
    os << static_cast<char>('a' + en_passant_target_x_);
    os << static_cast<char>('1' + en_passant_target_y_);
    os << ' ';
  }
  // halfmove clock
  os << halfmove_clock_;
  os << ' ';
  // fullmove counter
  os << fullmove_counter_;
  
  return os.str();
}

/* static */
void Position::ParseFen(const string& fen, Position* pos) {
  istringstream is(fen);
  
  // Parse the board string.
  int x = 0;
  int y = 7;
  int state = 0;
  char c;
  while (state == 0) {
    is.get(c);
    switch (c) {
      case 'k':
        pos->set_board(x, y, -6);
        ++x;
        break;
        
      case 'q':
        pos->set_board(x, y, -5);
        ++x;
        break;
        
      case 'r':
        pos->set_board(x, y, -4);
        ++x;
        break;
        
      case 'b':
        pos->set_board(x, y, -3);
        ++x;
        break;
        
      case 'n':
        pos->set_board(x, y, -2);
        ++x;
        break;
        
      case 'p':
        pos->set_board(x, y, -1);
        ++x;
        break;
        
      case 'P':
        pos->set_board(x, y, 1);
        ++x;
        break;
        
      case 'N':
        pos->set_board(x, y, 2);
        ++x;
        break;
        
      case 'B':
        pos->set_board(x, y, 3);
        ++x;
        break;
        
      case 'R':
        pos->set_board(x, y, 4);
        ++x;
        break;
        
      case 'Q':
        pos->set_board(x, y, 5);
        ++x;
        break;
        
      case 'K':
        pos->set_board(x, y, 6);
        ++x;
        break;
        
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      {
        int n = static_cast<int>(c - '0');
        for (int i = 0; i < n; ++i) {
          pos->set_board(x, y, 0);
          ++x;
        }
      }
        break;
        
      case '/':
        x = 0;
        --y;
        break;
        
      case ' ':
        state = 1;
        break;
        
      default:
        // error!
        cerr << "Unknown piece char = " << c << endl;
        state = -1;
        break;
    }
    // TODO: error check!
  }
  
  // Parse side to move.
  while (state == 1) {
    is.get(c);
    switch (c) {
      case 'b':
        pos->side_ = -1;
        break;
        
      case 'w':
        pos->side_ = 1;
        break;
        
      case ' ':
        state = 2;
        break;
        
      default:
        // error!
        state = -1;
        break;
    }
  }
  
  // Parse castling ability.
  pos->can_castling_[0][0] = false;
  pos->can_castling_[0][1] = false;
  pos->can_castling_[1][0] = false;
  pos->can_castling_[1][1] = false;
  while (state == 2) {
    is.get(c);
    switch (c) {
      case '-':
        state = 3;
        break;
        
      case 'K':
        pos->can_castling_[0][1] = true;
        break;
        
      case 'Q':
        pos->can_castling_[0][0] = true;
        break;
        
      case 'k':
        pos->can_castling_[1][1] = true;
        break;
        
      case 'q':
        pos->can_castling_[1][0] = true;
        break;
        
      case ' ':
        state = 3;
        break;
        
      default:
        // error!
        state = -1;
        break;
    }
  }
  
  // Parse en passant target.
  while (state == 3) {
    is.get(c);
    switch (c) {
      case '-':
        pos->en_passant_target_x_ = -1;
        state = 4;
        break;
        
      case 'a':
        pos->en_passant_target_x_ = 0;
        break;
        
      case 'b':
        pos->en_passant_target_x_ = 1;
        break;
        
      case 'c':
        pos->en_passant_target_x_ = 2;
        break;
        
      case 'd':
        pos->en_passant_target_x_ = 3;
        break;
        
      case 'e':
        pos->en_passant_target_x_ = 4;
        break;
        
      case 'f':
        pos->en_passant_target_x_ = 5;
        break;
        
      case 'g':
        pos->en_passant_target_x_ = 6;
        break;
        
      case 'h':
        pos->en_passant_target_x_ = 7;
        break;
        
      case '1':
        pos->en_passant_target_y_ = 0;
        break;
        
      case '2':
        pos->en_passant_target_y_ = 1;
        break;
        
      case '3':
        pos->en_passant_target_y_ = 2;
        break;
        
      case '4':
        pos->en_passant_target_y_ = 3;
        break;
        
      case '5':
        pos->en_passant_target_y_ = 4;
        break;
        
      case '6':
        pos->en_passant_target_y_ = 5;
        break;
        
      case '7':
        pos->en_passant_target_y_ = 6;
        break;
        
      case '8':
        pos->en_passant_target_y_ = 7;
        break;
        
      case ' ':
        state = 4;
        break;
        
      default:
        // error!
        cerr << "Error at en passant target: " << c << endl;
        state = -1;
        break;
    }
  }
  
  // Parse halfmove clock.
  is >> pos->halfmove_clock_;
  
  // Parse fullmove counter.
  is >> pos->fullmove_counter_;
}

void Position::DoMove(const Move& m, Position* dst) {
  assert(m.from_x() >= 0 && m.from_x() < 8);
  assert(m.from_y() >= 0 && m.from_y() < 8);
  
  dst->PartialCopyFrom(*this);
  //*dst = *this;
  int p = dst->board_[m.from_x()][m.from_y()];
  // half move clock
  if (p == 1 || p == -1 || dst->board_[m.to_x()][m.to_y()] != 0) {
    dst->halfmove_clock_ = 0;
  } else {
    ++dst->halfmove_clock_;
  }
  // Clear from position.
  dst->board_[m.from_x()][m.from_y()] = 0;
  if (p == 1 && m.from_y() == 6) {
    // promote
    dst->board_[m.to_x()][m.to_y()] = m.piece();
  } else if (p == -1 && m.from_y() == 1) {
    // promote
    dst->board_[m.to_x()][m.to_y()] = m.piece();
  } else if (p == 6 && (m.to_x() - m.from_x()) == 2) {
    // white king-side_ castling
    //dst->board_[4][0] = 0;
    dst->board_[7][0] = 0;
    dst->board_[5][0] = 4;
    dst->board_[6][0] = 6;
  } else if (p == 6 && (m.from_x() - m.to_x()) == 2) {
    // white queen-side_ castling
    dst->board_[0][0] = 0;
    dst->board_[2][0] = 6;
    dst->board_[3][0] = 4;
  } else if (p == -6 && (m.to_x() - m.from_x()) == 2) {
    // black king-side_ castling
    //dst->board_[4][7] = 0;
    dst->board_[7][7] = 0;
    dst->board_[5][7] = -4;
    dst->board_[6][7] = -6;
  } else if (p == -6 && (m.from_x() - m.to_x()) == 2) {
    // black queen-side_ castling
    dst->board_[0][7] = 0;
    dst->board_[2][7] = -6;
    dst->board_[3][7] = -4;
  } else if (p == 1 &&
             m.from_x() != m.to_x() &&
             dst->board_[m.to_x()][m.to_y()] == 0) {
    // white enpassant
    dst->board_[m.to_x()][m.to_y() - 1] = 0;
    dst->board_[m.to_x()][m.to_y()] = p;
  } else if (p == -1 &&
             m.from_x() != m.to_x() &&
             dst->board_[m.to_x()][m.to_y()] == 0) {
    // black enpassant
    dst->board_[m.to_x()][m.to_y() + 1] = 0;
    dst->board_[m.to_x()][m.to_y()] = p;
  } else {
    dst->board_[m.to_x()][m.to_y()] = p;
  }
  dst->side_ = -(dst->side_);
  if (dst->side_ > 0) {
    ++(dst->fullmove_counter_);
  }
  
  // Set en passant target.
  if (p == 1 && (m.to_y() - m.from_y()) == 2) {
    // a double push of a white pawn
    dst->en_passant_target_x_ = m.from_x();
    dst->en_passant_target_y_ = m.from_y() + 1;
  } else if (p == -1 && (m.to_y() - m.from_y()) == -2) {
    // a double push of a black pawn
    dst->en_passant_target_x_ = m.from_x();
    dst->en_passant_target_y_ = m.from_y() - 1;
  } else {
    dst->en_passant_target_x_ = -1;
    //dst->en_passant_target_y = -1;
  }
  
  //dst->next_moves_.clear();
  // for castling
  if (p == -6) {
    dst->can_castling_[0][0] = false;
    dst->can_castling_[0][1] = false;
  } else if (p == 6) {
    dst->can_castling_[1][0] = false;
    dst->can_castling_[1][1] = false;
  }
  if (p == -4) {
    if (m.from_y() == 7) {
      if (m.from_x() == 0) {
        dst->can_castling_[0][0] = false;
      } else if (m.from_x() == 7) {
        dst->can_castling_[0][1] = false;
      }
    }
  } else if (p == 4) {
    if (m.from_y() == 0) {
      if (m.from_x() == 0) {
        dst->can_castling_[1][0] = false;
      } else if (m.from_x() == 7) {
        dst->can_castling_[1][1] = false;
      }
    }
  }
}

void Position::CalcMoves() {
  CalcIsUnderAttack();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      switch (board_[x][y] * side_) {
        case 1:
          CalcPawnMoves(x, y);
          break;
          
        case 2:
          CalcKnightMoves(x, y);
          break;
          
        case 3:
          CalcBishopMoves(x, y);
          break;
          
        case 4:
          CalcRookMoves(x, y);
          break;
          
        case 5:
          CalcQueenMoves(x, y);
          break;
          
        case 6:
          CalcKingMoves(x, y);
          break;
          
        default:
          // error
          break;
      }
    }
  }
}

// Calculates pawn moves.
// Assumes the piece of the given (x, y) is a pawn.
void Position::CalcPawnMoves(int x, int y) {
  // basic move
  int to_y = y + side_;
  if (to_y >= 0 && to_y < 8) {
    if (board_[x][to_y] == 0) {
      AddPawnMoves(x, y, x, to_y);
      // initial 2 move
      if ((y == 1 && side_ == 1) ||
          (y == 6 && side_ == -1)) {
        to_y += side_;
        if (board_[x][to_y] == 0) {
          AddMove(x, y, x, to_y, 0);
        }
        to_y -= side_;
      }
    }
    // capture
    int to_x = x - 1;
    if (to_x >= 0 &&
        (side_ * board_[to_x][to_y] < 0 || // foe's piece
         (to_x == en_passant_target_x_ && to_y == en_passant_target_y_))) {
          AddPawnMoves(x, y, to_x, to_y);
        }
    to_x = x + 1;
    if (to_x < 8 &&
        (side_ * board_[to_x][to_y] < 0 || // foe's piece
         (to_x == en_passant_target_x_ && to_y == en_passant_target_y_))) {
          AddPawnMoves(x, y, to_x, to_y);
        }
  }
}

void Position::AddPawnMoves(int from_x, int from_y, int to_x, int to_y) {
  if (to_y == 0 && side_ == -1) {
    // promote
    for (int p = 2; p < 6; ++p) {
      AddMove(from_x, from_y, to_x, to_y, -p);
    }
  } else if (to_y == 7 && side_ == 1) {
    // promote
    for (int p = 2; p < 6; ++p) {
      AddMove(from_x, from_y, to_x, to_y, p);
    }
  } else {
    AddMove(from_x, from_y, to_x, to_y, 0);
  }
}

/**
 * Adds a move if it is valid.
 * Returns 1 if it is valid and the to-position is blank. Otherwise 0.
 */
int Position::AddMove(int from_x, int from_y, int to_x, int to_y, int piece) {
  if (to_x >= 0 && to_x < 8 &&
      to_y >= 0 && to_y < 8) {
    int t = board_[to_x][to_y] * side_;
    if (t <= 0) {
      Move m(from_x, from_y, to_x, to_y, piece);
      // check if the move m makes our king under attack.
      Position next_pos;
      DoMove(m, &next_pos);
      next_pos.side_ = -(next_pos.side_);
      if (!next_pos.IsCheck()) {
        next_moves_.push_back(m);
      }
      if (t == 0) {
        return 1;
      }
    }
  }
  return 0;
}

void Position::CalcKnightMoves(int x, int y) {
  AddMove(x, y, x + 1, y + 2, 0);
  AddMove(x, y, x + 2, y + 1, 0);
  AddMove(x, y, x + 2, y - 1, 0);
  AddMove(x, y, x + 1, y - 2, 0);
  AddMove(x, y, x - 1, y - 2, 0);
  AddMove(x, y, x - 2, y - 1, 0);
  AddMove(x, y, x - 2, y + 1, 0);
  AddMove(x, y, x - 1, y + 2, 0);
}

void Position::CalcBishopMoves(int x, int y) {
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x - i, y - i, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x + i, y - i, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x + i, y + i, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x - i, y + i, 0)) {
      break;
    }
  }
}

void Position::CalcRookMoves(int x, int y) {
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x - i, y, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x + i, y, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x, y - i, 0)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!AddMove(x, y, x, y + i, 0)) {
      break;
    }
  }
}

void Position::CalcQueenMoves(int x, int y) {
  CalcBishopMoves(x, y);
  CalcRookMoves(x, y);
}

void Position::CalcKingMoves(int x, int y) {
  AddMove(x, y, x    , y + 1, 0);
  AddMove(x, y, x + 1, y + 1, 0);
  AddMove(x, y, x + 1, y    , 0);
  AddMove(x, y, x + 1, y - 1, 0);
  AddMove(x, y, x    , y - 1, 0);
  AddMove(x, y, x - 1, y - 1, 0);
  AddMove(x, y, x - 1, y    , 0);
  AddMove(x, y, x - 1, y + 1, 0);
  // castling
  int s = side_ > 0;
  int cy = s ? 0 : 7;
  if (can_castling_[s][0] &&
      board_[1][cy] == 0 &&
      board_[2][cy] == 0 &&
      board_[3][cy] == 0 &&
      !is_under_attack_[2][cy] &&
      !is_under_attack_[3][cy] &&
      !is_under_attack_[4][cy]) {
    // Queen's side_
    AddMove(x, cy, x - 2, cy, 0);
  } else if (can_castling_[s][1] &&
             board_[5][cy] == 0 &&
             board_[6][cy] == 0 &&
             !is_under_attack_[4][cy] &&
             !is_under_attack_[5][cy] &&
             !is_under_attack_[6][cy]) {
    // King's side_
    AddMove(x, cy, x + 2, cy, 0);
  }
}

bool Position::IsCheck() {
  CalcIsUnderAttack();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      if (board_[x][y] == 6 * side_) {
        return is_under_attack_[x][y];
      }
    }
  }
  return false;  // make a compiler happy.
}

void Position::CalcIsUnderAttack() {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      is_under_attack_[x][y] = false;
    }
  }
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      switch (board_[x][y] * side_) {
        case -1:
          CalcIsUnderAttackByPawn(x, y);
          break;
          
        case -2:
          CalcIsUnderAttackByKnight(x, y);
          break;
          
        case -3:
          CalcIsUnderAttackByBishop(x, y);
          break;
          
        case -4:
          CalcIsUnderAttackByRook(x, y);
          break;
          
        case -5:
          CalcIsUnderAttackByQueen(x, y);
          break;
          
        case -6:
          CalcIsUnderAttackByKing(x, y);
          break;
          
        default:
          // error
          break;
      }
    }
  }
}

void Position::CalcIsUnderAttackByPawn(int x, int y) {
  int to_y = y - side_;
  if (to_y >= 0 && to_y < 8) {
    // capture
    int to_x = x - 1;
    if (to_x >= 0) {
      is_under_attack_[to_x][to_y] = true;
    }
    to_x = x + 1;
    if (to_x < 8) {
      is_under_attack_[to_x][to_y] = true;
    }
  }
}

void Position::CalcIsUnderAttackByKnight(int x, int y) {
  SetIsUnderAttack(x + 1, y + 2);
  SetIsUnderAttack(x + 2, y + 1);
  SetIsUnderAttack(x + 2, y - 1);
  SetIsUnderAttack(x + 1, y - 2);
  SetIsUnderAttack(x - 1, y - 2);
  SetIsUnderAttack(x - 2, y - 1);
  SetIsUnderAttack(x - 2, y + 1);
  SetIsUnderAttack(x - 1, y + 2);
}

void Position::CalcIsUnderAttackByBishop(int x, int y) {
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x - i, y - i)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x + i, y - i)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x + i, y + i)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x - i, y + i)) {
      break;
    }
  }
}

void Position::CalcIsUnderAttackByRook(int x, int y) {
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x - i, y)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x + i, y)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x, y - i)) {
      break;
    }
  }
  for (int i = 1; i < 8; ++i) {
    if (!SetIsUnderAttack(x, y + i)) {
      break;
    }
  }
}

void Position::CalcIsUnderAttackByQueen(int x, int y) {
  CalcIsUnderAttackByBishop(x, y);
  CalcIsUnderAttackByRook(x, y);
}

void Position::CalcIsUnderAttackByKing(int x, int y) {
  SetIsUnderAttack(x    , y + 1);
  SetIsUnderAttack(x + 1, y + 1);
  SetIsUnderAttack(x + 1, y    );
  SetIsUnderAttack(x + 1, y - 1);
  SetIsUnderAttack(x    , y - 1);
  SetIsUnderAttack(x - 1, y - 1);
  SetIsUnderAttack(x - 1, y    );
  SetIsUnderAttack(x - 1, y + 1);
}

/**
 * Set if the specified tile is under attack.
 * Returns 1 if it is open. Otherwise 0.
 */
int Position::SetIsUnderAttack(int x, int y) {
  if (x >= 0 && x < 8 && y >= 0 && y < 8) {
    is_under_attack_[x][y] = true;
    if (board_[x][y] == 0) {
      return true;
    }
  }
  return false;
}

bool Position::IsValidMove(const Move& m) {
  return (find(next_moves_.begin(), next_moves_.end(), m) != next_moves_.end());
}


// RandomPlayer
bool RandomPlayer::NextMove(Position& pos, Move* next_move) {
  pos.CalcMoves();
  if (pos.next_moves().empty()) {
    return false;
  }
  int i = rand() % pos.next_moves().size();
  *next_move = pos.next_moves()[i];
  return true;
}

// MinMaxPlayer

MinMaxPlayer::MinMaxPlayer(int max_depth)
: max_depth_(max_depth) {}

bool MinMaxPlayer::NextMove(Position& pos, Move* next_move) {
  count = 0;
  bool ret = CalcNextMove(pos, max_depth_, next_move, &last_score);
  //printf("count = %d\n", count);
  return ret;
}

bool MinMaxPlayer::CalcNextMove(Position& pos, int depth, Move* next_move, int* score) {
  ++count;
  if (depth < max_depth_) {
    pos.CalcMoves();
  }
  if (pos.next_moves().empty()) {
    if (pos.IsCheck()) {
      // loose
      *score = -2000;
    } else {
      // stale mate
      *score = -1000;
    }
    return false;
  }
  if (depth == 0) {
    *score = CalcScore(pos);
    return true;
  }
  *score = -10000;
  for (vector<Move>::const_iterator it = pos.next_moves().begin(); it != pos.next_moves().end(); ++it) {
    Position next_pos;
    pos.DoMove(*it, &next_pos);
    Move next_next_move;
    int next_score;
    CalcOpponentNextMove(next_pos, depth - 1, &next_next_move, &next_score);
    /*
     if (depth == max_depth_) {
     it->Print();
     printf(" ");
     next_next_move.Print();
     printf(" %d\n", next_score);
     }
     */
    if (next_score > *score) {
      *next_move = *it;
      *score = next_score;
    }
  }
  return true;
}

bool MinMaxPlayer::CalcOpponentNextMove(Position& pos, int depth, Move* next_move, int* score) {
  ++count;
  pos.CalcMoves();
  if (pos.next_moves().empty()) {
    if (pos.IsCheck()) {
      // win
      *score = 2000;
    } else {
      // stale mate
      *score = -1000;
    }
    return false;
  }
  if (depth == 0) {
    *score = CalcScore(pos);
    return true;
  }
  *score = 10000;
  for (vector<Move>::const_iterator it = pos.next_moves().begin(); it != pos.next_moves().end(); ++it) {
    Position next_pos;
    pos.DoMove(*it, &next_pos);
    Move next_next_move;
    int next_score;
    CalcNextMove(next_pos, depth - 1, &next_next_move, &next_score);
    if (next_score < *score) {
      *next_move = *it;
      *score = next_score;
    }
  }
  return true;
}

int MinMaxPlayer::CalcScore(Position& pos) {
  const static int PIECE_SCORE[] = {
    200, 9, 5, 3, 3, 1, 0, -1, -3, -3, -5, -9, -200
  };
  int score = 0;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      score += PIECE_SCORE[pos.get_board(x, y) + 6];
    }
  }
  return score;
}