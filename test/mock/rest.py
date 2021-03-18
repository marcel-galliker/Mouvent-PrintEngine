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
        return web.json_response([x.no for x in self.boards[name]])
    async def get_board(self, request):
        b = self.boards[request.match_info['dev']][int(request.match_info['number'])]
        return web.json_response(b.config)

    async def post_board(self, request):
        config = self.boards[request.match_info['dev']][int(request.match_info['number'])].config
        config = config | await request.json()
        self.boards[request.match_info['dev']][int(request.match_info['number'])].config = config
        #self.boards[request.match_info['dev']][int(request.match_info['number'])].config.update(await request.json())
        return web.json_response(self.boards[request.match_info['dev']][int(request.match_info['number'])].config)


def create_app(boards):
        app = web.Application()
        handler = Handler(boards)

        app.router.add_get('/', handler.root)
        app.router.add_get('/{dev}', handler.board_list)
        app.router.add_get(r'/{dev}/{number:\d+}', handler.get_board)
        app.router.add_post(r'/{dev}/{number:\d+}', handler.post_board)

        rootDir = os.path.dirname(os.path.realpath(__file__))
        UIDir = rootDir + '\\UI'
        app.router.add_static('/UI/', path=UIDir)

        return app
