from aiohttp import web
import json
import os

class Handler:
    def __init__(self, boards):
        self.boards = boards

    async def root(self, request):
        return web.Response(
            body=json.dumps(list(self.boards.keys())).encode('utf-8'),
            content_type='application/json')
    async def board_list(self, request):
        name = request.match_info['dev']
        if name!="favicon.ico":
            return web.json_response([x.no for x in self.boards[name]])
    async def get_board(self, request):
        b = self.boards[request.match_info['dev']][int(request.match_info['number'])]
        return web.json_response(b.config)

    async def post_board(self, request):
       self.boards[request.match_info['dev']][int(request.match_info['number'])].config.update(await request.json())
       return web.json_response(self.boards[request.match_info['dev']][int(request.match_info['number'])].config)


def create_app(boards):
        app = web.Application()
        handler = Handler(boards)

        root_dir = os.path.dirname(os.path.realpath(__file__))

        app.router.add_get('/', handler.root)
        try:
            app.router.add_static('/UI/', path=os.path.join(root_dir, "UI"))
        except ValueError: # on standalone mock this folder does not exists
            app.router.add_static('/UI/', path=os.path.join(root_dir, "../../UI"))

        app.router.add_get('/{dev}', handler.board_list)
        app.router.add_get(r'/{dev}/{number:\d+}', handler.get_board)
        app.router.add_post(r'/{dev}/{number:\d+}', handler.post_board)


        return app
