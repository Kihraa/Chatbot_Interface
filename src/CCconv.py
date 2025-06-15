#!/Users/david/Desktop/Progra_s/KeyInterface/myenv/bin/python
import openai # openai v1.0.0+
apiKey = open('key.txt', 'r').read().strip()
model1 = "llama3.1:8b"
model2 = "llama3.1:8b"
client = openai.OpenAI(api_key=apiKey,base_url="https://litellm.sph-prod.ethz.ch/v1")

intro = ""
chat1 = []
chat2 = []

def runResponse(chat, model, question):
    chat.append({"role": "user", "content":question})
    response = client.chat.completions.create(model=model, messages = chat).choices[0].message.content
    chat.append({"role": "assistant", "content":response})
    return response

def stepQ():
    order = input("------------------------")    #Continue (enter), modify (Not implemented) or exit (quit)
    if order == "quit":
        return False
    return True

if stepQ():
    response = runResponse(chat1, model1, intro)
    print("Bob: " + response)
    if stepQ():
        response = runResponse(chat2, model2, intro + "\n" + response)
        print("Anna: " + response)
        while stepQ():
            response = runResponse(chat1, model1, response)
            print("Bob: " + response)
            if stepQ():
                response = runResponse(chat2, model2, response)
                print("Anna: " + response)
            else: break
