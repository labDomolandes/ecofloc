# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

from dash import dcc, html, callback_context
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output, State, ALL
from dash.exceptions import PreventUpdate
import json
import os
#To fork floc for each available hardware component
import subprocess
import multiprocessing



#To fork floc instances

from concurrent.futures import ProcessPoolExecutor, as_completed
import subprocess


###################################################################################
##       PANEL LAYOUT 
###################################################################################


def floc_daemon_layout():
    initial_options = ['cpu', 'ram', 'sd', 'nic']  # Full list of options for the first selector
    return html.Div([

    ###################################################################################

        html.H4("1) What do you want to monitor?"),  # Main header
        
        dcc.Dropdown(
            id='pid-app-selector',
            options=[
                {'label': 'Single Process PID', 'value': 'pid'},
                {'label': 'Single Application Name', 'value': 'app_name'},
                {'label': 'Activity Applications', 'value': 'activity_name'}  
            ],
            value='pid',  # Default selection
            clearable=False,
            style={'width': '47%'}
        ),
        html.Div(id='pid-app-input', children=[]),  # Dynamic input for PID, application name, or activity name
    
    ###################################################################################
        
        html.H4("2) Select the hardware devices to analyze "),  
        
        html.Div(id='dynamic-hardware-selectors', children=[
            hardware_selector(0, initial_options)  # Initial hardware selector
        ]),
        html.Button("Add Hardware", id="add-hardware", className="button", n_clicks=0),  # Button for adding more hardware selectors
        
    ###################################################################################
    
        html.H4("3) Now select the measurement interval and timeout"),

        html.Div([
            html.Label("Execution Interval (ms):"),
            dcc.Input(id='interval-input', type='number', value=1000, placeholder="Enter interval in milliseconds"),
            dbc.Button("?", id="popover-interval-target", color="warning", className="mb-1"),
            dbc.Popover(
                [
                    dbc.PopoverHeader("Execution Interval Information", className="custom-popover-header"),
                    dbc.PopoverBody(
                        "This determines how often FLOC captures hardware load data to calculate power usage. ",
                        #"A smaller interval increases CPU usage but captures brief processes more accurately.",
                        className="custom-popover-body" #LOOK AT THE CSS IN THE ASSETS FOLDER 
                    ),
                ],
                id="popover-interval",
                target="popover-interval-target",
                trigger="hover",
                style={"maxWidth": "400px", "border": "1px solid black", "boxShadow": "3px 3px 3px rgba(0,0,0,0.2)"}
            ),
        ]),

        html.Div([
            html.Label("Execution Time (s):"),
            dcc.Input(id='time-input', type='number', value=40, placeholder="Enter time in seconds"),
            dbc.Button("?", id="popover-time-target", color="warning", className="mb-1"),
            dbc.Popover(
                [
                    dbc.PopoverHeader("Execution Time Information", className="custom-popover-header"),
                    dbc.PopoverBody(
                        "This is the overall time FLOC analyzes something."
                        "Enter a negative value for an endless analysis.",
                        className="custom-popover-body" #LOOK AT THE CSS IN THE ASSETS FOLDER
                    ),
                ],
                id="popover-time",
                target="popover-time-target",
                trigger="hover",
                style={"maxWidth": "400px", "border": "1px solid black", "boxShadow": "3px 3px 3px rgba(0,0,0,0.2)"}
            ),
        ]),

        ###################################################################################


        html.H4("4) Select additional options"),

        html.Div([
            html.Div([
                dcc.Checklist(
                    id='options-checklist-dynamic',
                    options=[{'label': 'Measure Intermittent Applications', 'value': 'dynamic'}],
                    value=[],
                    inputStyle={'marginRight': '10px', 'cursor': 'pointer'},
                    labelStyle={'display': 'inline-block'}
                ),
                dbc.Button("?", id="popover-dynamic-mode-target", color="warning", className="ms-2 d-inline-block"),
                dbc.Popover(
                    [
                        dbc.PopoverHeader("Dynamic Mode Information",className="custom-popover-header"),
                        dbc.PopoverBody(
                            "That means that FLOC monitors apps that aren't continuously running.",
                            className="custom-popover-body"
                        ),
                    ],
                    id="popover-dynamic-mode",
                    target="popover-dynamic-mode-target",
                    trigger="hover",
                    style={"maxWidth": "400px"}
                ),
            ], style={'display': 'flex', 'alignItems': 'center'}),

            html.Div([
                dcc.Checklist(
                    id='options-checklist-export-json',
                    options=[{'label': 'Export metrics to CSV', 'value': 'export_json'}],
                    value=[],
                    inputStyle={'marginRight': '10px', 'cursor': 'pointer'},
                    labelStyle={'display': 'inline-block'}
                ),
                dbc.Button("?", id="popover-export-json-target", color="warning", className="ms-2 d-inline-block"),
                dbc.Popover(
                    [
                        dbc.PopoverHeader("Export to JSON Information",className="custom-popover-header"),
                        dbc.PopoverBody(
                            "When enabled, FLOC exports power metrics to a CSV file at the specified interval. "
                            "WARN: setting a very short interval may result in a high disk load.",
                            className="custom-popover-body"
                        ),
                    ],
                    id="popover-export-json",
                    target="popover-export-json-target",
                    trigger="hover",
                    style={"maxWidth": "400px"}
                ),
            ], style={'display': 'flex', 'alignItems': 'center'}),
        ]),





        
        ###################################################################################
    
        html.H4("5) GO! "),  

        html.Button('Start Monitoring', id='start-monitoring', className="button", n_clicks=0),  # Button to start the monitoring process
        html.Div(id='output-console')  # Area to display outputs from the monitoring command
    ])



###################################################################################
##      CORE FUNCTIONS
###################################################################################


# Create a new hardware selector dropdown
def hardware_selector(index, available_options):
    return html.Div([
        dcc.Dropdown(
            id={'type': 'hardware-component', 'index': index},
            options=[{'label': option, 'value': option} for option in available_options],
            value=available_options[0] if available_options else None,
            clearable=False,
            style={'width': '300px'}
        )
    ], style={'margin': '10px'})



# Launch a floc instance for a hardware component -> forked in callback below 

def execute_command(args):
    hardware, pid_or_app_value, interval, time, mode, dynamic_active, export_json_active = args
    # Adjusting command based on mode, dynamic_active, and export_json_active
    flag = '-n' if mode == 'app_name' else '-p'
    
    command = f"floc --{hardware} {flag} {pid_or_app_value} -i {interval} -t {time}"
    
    if dynamic_active:
        command += " -d true"
    
    #TODO -> Set the path in a config file.
    if export_json_active:
        home_dir = os.getenv('HOME')
        metrics_dir = os.path.join(home_dir, 'FLOC_METRICS')
        os.makedirs(metrics_dir, exist_ok=True)
        filename = f"{hardware}.csv"
        filepath = os.path.join(metrics_dir, filename)
        command += f" -f {filepath}"
    
    try:
        print(f"Executing command: {command}")
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        print(f"Command Output: {result.stdout}")
        return f"Command: {command} Output: {result.stdout}"
    except Exception as e:
        return f"Command: {command} Error: {str(e)}"





###################################################################################
##      DECORATORS AND CALLBACKS 
###################################################################################

# Remember, in decorators, input and states are inputs in the callbacks' functions :)

def register_floc_daemon_callbacks(app):
    # Callback to handle clicks on the "Add Hardware" button
    @app.callback(
        Output('dynamic-hardware-selectors', 'children'),
        Input('add-hardware', 'n_clicks'),
        State('dynamic-hardware-selectors', 'children'),
        State({'type': 'hardware-component', 'index': ALL}, 'value')
    )
    def add_hardware_selector(n_clicks, children, selected_values):
        if n_clicks > 0:
            all_options = {'cpu', 'ram', 'sd', 'nic'}
            selected_set = set(selected_values)
            available_options = list(all_options - selected_set)
            if available_options:
                new_index = len(children)
                new_element = hardware_selector(new_index, available_options)
                children.append(new_element)
            return children
        raise PreventUpdate

    # Callback to dynamically update the input field based on PID/App Name selection
    @app.callback(
        Output('pid-app-input', 'children'),
        Input('pid-app-selector', 'value'),
        [State('stored-selected-process', 'data'),
         State('stored-selected-activity', 'data')]  # Add the state of stored-selected-activity
    )
    def update_input(selected_option, stored_process_data, stored_activity_data):
        pid = stored_process_data.get('pid', '') if stored_process_data else ''
        app_name = stored_process_data.get('name', '') if stored_process_data else ''
        activity_name = ', '.join(stored_activity_data['elements']) if stored_activity_data and 'elements' in stored_activity_data else ''  # Handle activity name

        print(activity_name)

        if selected_option == 'pid':
            return dcc.Input(id={'type': 'dynamic-input', 'id': 'pid-input'}, type='text', value=pid, placeholder="Enter PID")
        elif selected_option == 'app_name':
            return dcc.Input(id={'type': 'dynamic-input', 'id': 'app-input'}, type='text', value=app_name, placeholder="Enter Application Name")
        elif selected_option == 'activity_name':
            return dcc.Input(id={'type': 'dynamic-input', 'id': 'activity-input'}, type='text', value=activity_name, placeholder="Enter Activity Name")

    # The PID Value from the system monitor  -> (only PID as it is the default value)
    @app.callback(
        Output({'type': 'dynamic-input', 'id': 'pid-input'}, 'value'),
        Input('stored-selected-process', 'data'),  # defined in system_monitor.py file
        prevent_initial_call=True
    )
    def update_pid_input(stored_data):
        if stored_data:
            pid = stored_data.get('pid', '')
            return pid  # Update only pid-input
        return ''  # Return empty string if no data

    # THE START MONITORING BUTTON ->  FLOC TRIGGER
    @app.callback(
        Output('output-console', 'children'),
        Input('start-monitoring', 'n_clicks'),
        State({'type': 'hardware-component', 'index': ALL}, 'value'),
        State('pid-app-selector', 'value'),
        State('interval-input', 'value'),
        State('time-input', 'value'),
        State({'type': 'dynamic-input', 'id': ALL}, 'value'),
        State('options-checklist-dynamic', 'value'),
        State('options-checklist-export-json', 'value')
    )
    def launch_monitoring(n_clicks, hardware_list, pid_or_app, interval, time, dynamic_inputs, dynamic_mode, export_json_mode):
        if n_clicks > 0:
            pid_or_app_value = dynamic_inputs[0]
            dynamic_active = 'dynamic' in dynamic_mode
            export_json_active = 'export_json' in export_json_mode

            if pid_or_app == 'activity_name':
                activity_names = pid_or_app_value.split(', ')
                #FORK APPS IN THE ACTIVITY -> ONLY LOOPS THE LIST AN FORK THE FUNCTION
                args = [(hardware, activity_name, interval, time, 'app_name', dynamic_active, export_json_active) for hardware in hardware_list for activity_name in activity_names]
            else:
                args = [(hardware, pid_or_app_value, interval, time, pid_or_app, dynamic_active, export_json_active) for hardware in hardware_list]

            with multiprocessing.Pool() as pool:
                results = pool.map(execute_command, args)
                commands = list(results)

            return html.Pre(json.dumps(commands, indent=2))
        raise PreventUpdate