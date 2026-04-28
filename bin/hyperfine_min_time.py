import json
import sys

def get_min_time_from_json(file_path):
    try:
        with open(file_path, 'r') as file:
            data = json.load(file)
            # Assuming the structure is as provided and consistent
            min_time = data['results'][0]['min']
            return min_time
    except FileNotFoundError:
        print("File not found. Please check the filename or path.")
    except json.JSONDecodeError:
        print("Error decoding JSON. Please check the file format.")
    except KeyError:
        print("Key not found in JSON. Please check the JSON structure.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <json_file>")
        sys.exit(1)

    filename = sys.argv[1]
    min_time = get_min_time_from_json(filename)
    if min_time is not None:
        print(f"{min_time}")

