#!/Users/david/Desktop/Progra_s/KeyInterface/myenv/bin/python
import openai # openai v1.0.0+
import json
import copy
apiKey = open('key.txt', 'r').read().strip()
model = "llama3.1:8b"

chat = []

def runResponse(chat, question):
    chat.append({"role": "user", "content":question})
    client = openai.OpenAI(api_key=apiKey,base_url="https://litellm.sph-prod.ethz.ch/v1")
    return client.chat.completions.create(model=model, messages = chat).choices[0].message.content

while True:
    question = input("You: ")
    if question == "quit":
        break
    if question == "load":
        filename = input("Name of the file to read: ")
        chat = json.load(open(filename, 'r'))
        print("Chat importet.")
        continue
    if question == "save":
        if not chat:
            print("No chat to save, write quit to exit!")
            continue
        question = "Based on our conversation so far, generate a concise, descriptive filename (without spaces, 3-5 words connected by underscores) that captures the main topic or theme. Respond with ONLY the filename and nothing else. Example format: topic_description_keywords"
        json.dump(chat, open(((runResponse(copy.deepcopy(chat), question).choices[0].message.content)+".json"), 'w'), indent=2)
        break
    response = runResponse(chat, question)
    chat.append({"role": "assistant", "content":response})
    print(model + ": " + response)
