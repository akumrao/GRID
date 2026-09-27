python3 -m http.server 8080

http://localhost:8080/test.html








python3 -c "
import asyncio, websockets, json
async def test():
    url = 'wss://dabwulkpyquthvearbfw.supabase.co/realtime/v1/websocket?apikey=sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu&vsn=1.0.0'
    async with websockets.connect(url) as ws:
        print('Connected! Paste your phx_join JSON payload below and press Enter:')
        while True:
            try:
                # Listen for user terminal input or background server events concurrently
                print('>>> ', end='', flush=True)
                msg = input()
                await ws.send(msg)
                res = await ws.recv()
                print(f'\n[Server Received]:\n{res}\n')
            except Exception as e:
                print('Session ended:', e)
                break
asyncio.run(test())
"








curl -i -N \
  -H "Connection: Upgrade" \
  -H "Upgrade: websocket" \
  -H "Host: dabwulkpyquthvearbfw.supabase.co" \
  -H "Origin: http://localhost" \
  -H "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==" \
  -H "Sec-WebSocket-Version: 13" \
  "https://supabase.co" \
  --data "$PAYLOAD"



echo '{"topic":"realtime:public:room:arvind","event":"phx_join","payload":{"config":{"broadcast":{"self":true,"ack":true},"presence":{"enabled":false},"private":false}},"ref":"1","join_ref":"1"}' | curl --include \
     --no-buffer \
     --header "Connection: Upgrade" \
     --header "Upgrade: websocket" \
     --header "Host: dabwulkpyquthvearbfw.supabase.co" \
     --header "Origin: http://localhost" \
     --header "Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==" \
     --header "Sec-WebSocket-Version: 13" \
     "https://supabase.co"





curl -i -N \
  -H "Connection: Upgrade" \
  -H "Upgrade: websocket" \
  -H "Host: dabwulkpyquthvearbfw.supabase.co" \
  -H "Origin: http://localhost" \
  -H "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==" \
  -H "Sec-WebSocket-Version: 13" \
  "https://dabwulkpyquthvearbfw.supabase.co/realtime/v1/websocket?apikey=sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu&vsn=1.0.0"





python3 -c "
import asyncio
import websockets
import json

async def run_test():
    url = 'wss://dabwulkpyquthvearbfw.supabase.co/realtime/v1/websocket?apikey=sb_publishable_1V4Zaw720lrIPx8IqmuxmA_ri32Bdqu&vsn=1.0.0'
    
    print('[+] Connecting to Supabase Gateway...')
    async with websockets.connect(url) as ws:
        print('[+] Handshake successful! Connected.')
        
        # 1. Compile the payloads
        join_payload = {
            'topic': 'realtime:public:room:arvind',
            'event': 'phx_join',
            'payload': {'config': {'broadcast': {'self': True, 'ack': True}, 'presence': {'enabled': False}, 'private': False}},
            'ref': '1',
            'join_ref': '1'
        }
        
        broadcast_payload = {
            'topic': 'realtime:public:room:arvind',
            'event': 'broadcast',
            'payload': {'type': 'broadcast', 'event': 'message_sent', 'payload': {'text': 'Hello from automated pipeline!'}},
            'ref': '2',
            'join_ref': '1'
        }
        
        heartbeat_payload = {
            'topic': 'phoenix',
            'event': 'heartbeat',
            'payload': {},
            'ref': '100',
            'join_ref': None
        }

        # Background task to continuously print incoming server frames
        async def listen_loop():
            try:
                async for message in ws:
                    print(f'\n[Server Response] -> {message}')
            except websockets.exceptions.ConnectionClosed:
                pass

        listener = asyncio.create_task(listen_loop())

        # 2. Send payloads sequentially with delays to observe responses
        print('[->] Sending Channel Join Request...')
        await ws.send(json.dumps(join_payload))
        await asyncio.sleep(2)

        print('[->] Sending Message Broadcast...')
        await ws.send(json.dumps(broadcast_payload))
        await asyncio.sleep(2)

        print('[->] Sending Heartbeat Keep-Alive...')
        await ws.send(json.dumps(heartbeat_payload))
        await asyncio.sleep(3)
        
        # Clean up
        listener.cancel()
        print('\n[+] Test complete. Closing connection.')

asyncio.run(run_test())
"
