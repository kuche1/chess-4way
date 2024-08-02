
///
//////
/////////// include
//////
///

#include <iostream>
#include <vector>
#include <cassert>

///
//////
/////////// namespace
//////
///

using namespace std;

///
//////
/////////// macro: generic
//////
///

#define DBG(...) { \
    cout << "DBG: "; \
    cout << __VA_ARGS__; \
    cout << endl; \
}

#define ERR(...) { \
    cout << "ERROR:" << endl; \
    cout << __VA_ARGS__ << endl; \
    cout << "(" << endl; \
    cout << "    location:" << endl; \
    cout << "        file `" << __FILE__ << "`" << endl; \
    cout << "        line " << __LINE__ << endl; \
    cout << "        function `" << __FUNCTION__ << "`" << endl; \
    cout << "        commit id `" << COMMIT_ID << "`" << endl; \
    cout << ")" << endl; \
    cout << endl; \
    exit(1); \
}

///
//////
/////////// define: icon
//////
///

#define ICON_PAWN_WHITE   "♙"
#define ICON_KNIGHT_WHITE "♘"
#define ICON_BISHOP_WHITE "♗"
#define ICON_ROOK_WHITE   "♖"
#define ICON_QUEEN_WHITE  "♕"
#define ICON_KING_WHITE   "♔"

#define ICON_PAWN_BLACK   "♟︎"
#define ICON_KNIGHT_BLACK "♞"
#define ICON_BISHOP_BLACK "♝"
#define ICON_ROOK_BLACK   "♜"
#define ICON_QUEEN_BLACK  "♛"
#define ICON_KING_BLACK   "♚"

#define ICON_WARNING "⚠"

///
//////
/////////// declaration
//////
///

class Piece;

///
//////
/////////// function: input
//////
///

void input_enter(){
    string trash;
    getline(cin, trash);
}

///
//////
/////////// module: terminal escape code
//////
///

void disp_clear(){
    cout << "\033[H\033[J";
}

void disp_cur_set(int y, int x){
    cout << "\033[";
    cout << y + 1;
    cout << ";";
    cout << x + 1;
    cout << "H";
}

///
//////
/////////// module: tile
//////
///

class Tile{

    public:

        int y;
        int x;

        // horizontal/vertical
        Tile * neighbour_up = nullptr;
        Tile * neighbour_down = nullptr;
        Tile * neighbour_left = nullptr;
        Tile * neighbour_right = nullptr;
        // diagonal
        Tile * neighbour_upleft = nullptr;
        Tile * neighbour_upright = nullptr;
        Tile * neighbour_downleft = nullptr;
        Tile * neighbour_downright = nullptr;

        Piece * piece = nullptr;

};

///
//////
/////////// module: piece
//////
///

enum piece_type{
    PT_PAWN,
    PT_KNIGHT,
    PT_BISHOP,
    PT_ROOK,
    PT_QUEEN,
    PT_KING,
};

class Piece{

    public:

        string icon;

        int forward_y;
        // int forward_x = 0; // this only makes sense in 4way chess

        int owner; // which player owns the piece

        enum piece_type type;

        Tile * location;

        bool has_not_moved = true;

        vector<Tile *> get_valid_moves(){

            vector<Tile *> moves = {};

            switch(type){

                case PT_PAWN:{
                    moves = gen_valid_moves_pawn();
                }break;

                case PT_KNIGHT:{
                    moves = gen_valid_moves_knight();
                }break;

                case PT_BISHOP:{
                    moves = gen_valid_moves_bishop();
                }break;

                case PT_ROOK:{
                    moves = gen_valid_moves_rook();
                }break;

                case PT_QUEEN:{
                    moves = gen_valid_moves_bishop();
                    for(Tile * rook_move : gen_valid_moves_rook()){
                        moves.push_back(rook_move);
                    }
                }break;

                case PT_KING:{
                    moves = gen_valid_moves_king();
                }break;

            }

            return moves;

        }

        void move_to(Tile * tile){

            location->piece = nullptr;
            location = tile;

            if(tile->piece){

                if(tile->piece->type == PT_KING){
                    ERR("TODO implement win");
                }

                tile->piece->location = nullptr;
                delete tile->piece;
            }

            tile->piece = this;

            has_not_moved = false;

        }
    
    private:


        vector<Tile *> gen_valid_moves_pawn(){

            vector<Tile *> moves = {};

            if(forward_y == -1){

                if(location->neighbour_up && !location->neighbour_up->piece){

                    moves.push_back(location->neighbour_up);

                    if(has_not_moved && location->neighbour_up->neighbour_up && !location->neighbour_up->neighbour_up->piece){

                        moves.push_back(location->neighbour_up->neighbour_up);

                    }

                }

                if(location->neighbour_upleft && (location->neighbour_upleft->piece && location->neighbour_upleft->piece->owner != owner)){
                    moves.push_back(location->neighbour_upleft);
                }

                if(location->neighbour_upright && (location->neighbour_upright->piece && location->neighbour_upright->piece->owner != owner)){
                    moves.push_back(location->neighbour_upright);
                }

                // TODO en passant

                // TODO promotion if end of board reached

            }else if(forward_y == 1){

                if(location->neighbour_down && !location->neighbour_down->piece){

                    moves.push_back(location->neighbour_down);

                    if(has_not_moved && location->neighbour_down->neighbour_down && !location->neighbour_down->neighbour_down->piece){

                        moves.push_back(location->neighbour_down->neighbour_down);

                    }

                }

                if(location->neighbour_downleft && (location->neighbour_downleft->piece && location->neighbour_downleft->piece->owner != owner)){
                    moves.push_back(location->neighbour_downleft);
                }

                if(location->neighbour_downright && (location->neighbour_downright->piece && location->neighbour_downright->piece->owner != owner)){
                    moves.push_back(location->neighbour_downright);
                }

                // TODO en passant

                // TODO promotion if end of board reached

            }else{

                ERR("unreachable");

            }

            return moves;

        }

        vector<Tile *> gen_valid_moves_knight(){

            vector<Tile *> moves = {};

            if(location->neighbour_up){

                if(location->neighbour_up->neighbour_upleft){
                    if(!location->neighbour_up->neighbour_upleft->piece || (location->neighbour_up->neighbour_upleft->piece->owner != owner)){
                        moves.push_back(location->neighbour_up->neighbour_upleft);
                    }
                }

                if(location->neighbour_up->neighbour_upright){
                    if(!location->neighbour_up->neighbour_upright->piece || (location->neighbour_up->neighbour_upright->piece->owner != owner)){
                        moves.push_back(location->neighbour_up->neighbour_upright);
                    }
                }

            }

            if(location->neighbour_down){

                if(location->neighbour_down->neighbour_downleft){
                    if(!location->neighbour_down->neighbour_downleft->piece || (location->neighbour_down->neighbour_downleft->piece->owner != owner)){
                        moves.push_back(location->neighbour_down->neighbour_downleft);
                    }
                }

                if(location->neighbour_down->neighbour_downright){
                    if(!location->neighbour_down->neighbour_downright->piece || (location->neighbour_down->neighbour_downright->piece->owner != owner)){
                        moves.push_back(location->neighbour_down->neighbour_downright);
                    }
                }

            }

            if(location->neighbour_left){

                if(location->neighbour_left->neighbour_upleft){
                    if(!location->neighbour_left->neighbour_upleft->piece || (location->neighbour_left->neighbour_upleft->piece->owner != owner)){
                        moves.push_back(location->neighbour_left->neighbour_upleft);
                    }
                }

                if(location->neighbour_left->neighbour_downright){
                    if(!location->neighbour_left->neighbour_downright->piece || (location->neighbour_left->neighbour_downright->piece->owner != owner)){
                        moves.push_back(location->neighbour_left->neighbour_downright);
                    }
                }

            }

            if(location->neighbour_right){

                if(location->neighbour_right->neighbour_upright){
                    if(!location->neighbour_right->neighbour_upright->piece || (location->neighbour_right->neighbour_upright->piece->owner != owner)){
                        moves.push_back(location->neighbour_right->neighbour_upright);
                    }
                }

                if(location->neighbour_right->neighbour_downright){
                    if(!location->neighbour_right->neighbour_downright->piece || (location->neighbour_right->neighbour_downright->piece->owner != owner)){
                        moves.push_back(location->neighbour_right->neighbour_downright);
                    }
                }

            }

            return moves;

        }

        vector<Tile *> gen_valid_moves_bishop(){

            vector<Tile *> moves = {};

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_upleft;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_upright;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_downleft;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_downright;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            return moves;

        }

        vector<Tile *> gen_valid_moves_rook(){

            vector<Tile *> moves = {};

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_up;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_down;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_left;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            {
                Tile * pos = location;

                while(true){
                    pos = pos->neighbour_right;
                    if(!pos){
                        break;
                    }

                    if(pos->piece){
                        if(pos->piece->owner != owner){
                            moves.push_back(pos);
                        }

                        break;
                    }
                }
            }

            return moves;

        }

        vector<Tile *> gen_valid_moves_king(){

            vector<Tile *> moves = {};

            if(location->neighbour_up && (!location->neighbour_up->piece || location->neighbour_up->piece->owner != owner)){
                moves.push_back(location->neighbour_up);
            }

            if(location->neighbour_upright && (!location->neighbour_upright->piece || location->neighbour_upright->piece->owner != owner)){
                moves.push_back(location->neighbour_upright);
            }

            if(location->neighbour_right && (!location->neighbour_right->piece || location->neighbour_right->piece->owner != owner)){
                moves.push_back(location->neighbour_right);
            }

            if(location->neighbour_downright && (!location->neighbour_downright->piece || location->neighbour_downright->piece->owner != owner)){
                moves.push_back(location->neighbour_downright);
            }

            if(location->neighbour_down && (!location->neighbour_down->piece || location->neighbour_down->piece->owner != owner)){
                moves.push_back(location->neighbour_down);
            }

            if(location->neighbour_downleft && (!location->neighbour_downleft->piece || location->neighbour_downleft->piece->owner != owner)){
                moves.push_back(location->neighbour_downleft);
            }

            if(location->neighbour_left && (!location->neighbour_left->piece || location->neighbour_left->piece->owner != owner)){
                moves.push_back(location->neighbour_left);
            }

            if(location->neighbour_upleft && (!location->neighbour_upleft->piece || location->neighbour_upleft->piece->owner != owner)){
                moves.push_back(location->neighbour_upleft);
            }

            return moves;

        }

};

///
//////
/////////// module: board
//////
///

class Board{

    public:

        vector<Tile *> tiles = {};

        int player_turn = 0; // which player's turn is it

        Board(){

            if(tiles.size() != 0){
                ERR("already initialised");
            }

            for(int y=0; y<8; ++y){
                for(int x=0; x<8; ++x){

                    Tile * tile = new Tile{
                        .y = y,
                        .x = x,
                    };

                    tiles.push_back(tile);

                }
            }

            connect_neighbours();

            place_pieces();

        }

        ~Board(){

            for(Tile * tile : tiles){

                if(tile->piece){
                    delete tile->piece;
                    tile->piece = nullptr;
                }

                delete tile;

            }

        }

        void draw(){

            disp_clear();

            for(Tile * tile : tiles){

                int draw_y = tile->y * 2 + 1;
                int draw_x = tile->x * 4 + 2;

                { // draw borders (ineffeciently)

                    disp_cur_set(draw_y, draw_x - 2);
                    cout << "|";

                    disp_cur_set(draw_y, draw_x + 2);
                    cout << "|";

                    disp_cur_set(draw_y - 1, draw_x - 1);
                    cout << "---";

                    disp_cur_set(draw_y + 1, draw_x - 1);
                    cout << "---";

                }

                Piece * piece = tile->piece;
                if(!piece){
                    continue;
                }

                disp_cur_set(draw_y, draw_x);
                
                cout << piece->icon;
            }

            cout << endl;
            cout << endl;

        }

        void next_turn(){

            int player = player_turn;
            player_turn = !player_turn;

            bool moved_something = false;

            for(Tile * tile : tiles){

                Piece * piece = tile->piece;
                if(!piece){
                    continue;
                }

                if(piece->owner != player){
                    continue;
                }

                vector<Tile *> valid_moves = piece->get_valid_moves();

                if(valid_moves.size() <= 0){
                    continue;
                }

                piece->move_to(valid_moves.at(0));

                moved_something = true;

                break;

            }

            if(!moved_something){
                ERR("todo implement stalemates");
            }

        }

    private:

        pair<bool, ssize_t> calc_idx(int y, int x){
            if(y < 0 || y >= 8 || x < 0 || x >= 8){
                return {true, 0};
            }
            return {false, y * 8 + x};
        }

        Tile * get_tile_at(int y, int x){

            auto [fail_ci, idx] = calc_idx(y, x);
            if(fail_ci){
                return nullptr;
            }

            return tiles[idx];

        }

        void set_tile_at(int y, int x, Tile * tile){

            auto [fail_ci, idx] = calc_idx(y, x);
            if(fail_ci){
                ERR("invalid tile y=" << y << " x=" << x);
            }

            if(tiles[idx]){
                ERR("tile not empty y=" << y << " x=" << x);
            }

            tiles[idx] = tile;

        }

        void connect_neighbours(){

            for(Tile * tile : tiles){

                { // up
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_up = nei;
                    }
                }

                { // down
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_down = nei;
                    }
                }

                { // left
                    int nei_up_y = tile->y;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_left = nei;
                    }
                }

                { // right
                    int nei_up_y = tile->y;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_right = nei;
                    }
                }

                { // up left
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_upleft = nei;
                    }
                }

                { // up right
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_upright = nei;
                    }
                }

                { // down left
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_downleft = nei;
                    }
                }

                { // down right
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_downright = nei;
                    }
                }

            }

        }

        void place_pieces(){

            for(auto [icon, type, owner, forward_y, y, x_start, x_step] : {
                make_tuple(ICON_PAWN_BLACK,   PT_PAWN,   1, 1, 1, 0, 1), make_tuple(ICON_PAWN_WHITE,   PT_PAWN,   0, -1, 6, 0, 1),
                make_tuple(ICON_ROOK_BLACK,   PT_ROOK,   1, 1, 0, 0, 7), make_tuple(ICON_ROOK_WHITE,   PT_ROOK,   0, -1, 7, 0, 7),
                make_tuple(ICON_KNIGHT_BLACK, PT_KNIGHT, 1, 1, 0, 1, 5), make_tuple(ICON_KNIGHT_WHITE, PT_KNIGHT, 0, -1, 7, 1, 5),
                make_tuple(ICON_BISHOP_BLACK, PT_BISHOP, 1, 1, 0, 2, 3), make_tuple(ICON_BISHOP_WHITE, PT_BISHOP, 0, -1, 7, 2, 3),
                make_tuple(ICON_QUEEN_BLACK,  PT_QUEEN,  1, 1, 0, 3, 9), make_tuple(ICON_QUEEN_WHITE,  PT_QUEEN,  0, -1, 7, 3, 9),
                make_tuple(ICON_KING_BLACK,   PT_KING,   1, 1, 0, 4, 9), make_tuple(ICON_KING_WHITE,   PT_KING  , 0, -1, 7, 4, 9),
            }){

                for(int x=x_start; x<8; x+=x_step){

                    auto [fail_ci, idx] = calc_idx(y, x);
                    if(fail_ci){
                        ERR("invalid position y=" << y << " x=" << x);
                    }

                    Tile * tile = tiles[idx];

                    if(tile->piece){
                        ERR("there is already a piece at y=" << y << " x=" << x);
                    }

                    Piece * piece = new Piece{
                        .icon = icon,
                        .forward_y = forward_y,
                        .owner = owner,
                        .type = type,
                        .location = tile,
                    };

                    tile->piece = piece;

                }

            }

        }

};

///
//////
/////////// function: main
//////
///

int main(){

    Board * board = new Board;

    while(true){

        board->draw();

        input_enter();

        board->next_turn();

    }

    delete board;

    return 0;
}
