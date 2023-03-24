import debug from 'debug';
let preLogHook;
let postLogHook;
function theFunctionThroughWhichAllLogsPass(logger, ...args) {
    if (!debug.enabled(logger.namespace)) {
        return;
    }
    if (preLogHook) {
        preLogHook(logger.namespace, ...args);
    }
    logger(...args);
    if (postLogHook) {
        postLogHook(logger.namespace, ...args);
    }
}
export function createDebugLogger(name) {
    const logger = debug(name);
    return (...args) => theFunctionThroughWhichAllLogsPass(logger, ...args);
}
export function setPreDebugLogHook(fn) {
    preLogHook = fn;
}
export function setPostDebugLogHook(fn) {
    postLogHook = fn;
}
export function enableLogs(str) {
    debug.enable(str);
}
export function isLogEnabled(str) {
    return debug.enabled(str);
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiZGVidWcuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi9zcmMvbG9nL2RlYnVnLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sS0FBSyxNQUFNLE9BQU8sQ0FBQztBQUUxQixJQUFJLFVBQWtELENBQUM7QUFDdkQsSUFBSSxXQUFtRCxDQUFDO0FBRXhELFNBQVMsa0NBQWtDLENBQUMsTUFBVyxFQUFFLEdBQUcsSUFBVztJQUNyRSxJQUFJLENBQUMsS0FBSyxDQUFDLE9BQU8sQ0FBQyxNQUFNLENBQUMsU0FBUyxDQUFDLEVBQUU7UUFDcEMsT0FBTztLQUNSO0lBQ0QsSUFBSSxVQUFVLEVBQUU7UUFDZCxVQUFVLENBQUMsTUFBTSxDQUFDLFNBQVMsRUFBRSxHQUFHLElBQUksQ0FBQyxDQUFDO0tBQ3ZDO0lBQ0QsTUFBTSxDQUFDLEdBQUcsSUFBSSxDQUFDLENBQUM7SUFDaEIsSUFBSSxXQUFXLEVBQUU7UUFDZixXQUFXLENBQUMsTUFBTSxDQUFDLFNBQVMsRUFBRSxHQUFHLElBQUksQ0FBQyxDQUFDO0tBQ3hDO0FBQ0gsQ0FBQztBQUVELE1BQU0sVUFBVSxpQkFBaUIsQ0FBQyxJQUFZO0lBQzVDLE1BQU0sTUFBTSxHQUFHLEtBQUssQ0FBQyxJQUFJLENBQUMsQ0FBQztJQUMzQixPQUFPLENBQUMsR0FBRyxJQUFXLEVBQUUsRUFBRSxDQUFDLGtDQUFrQyxDQUFDLE1BQU0sRUFBRSxHQUFHLElBQUksQ0FBQyxDQUFDO0FBQ2pGLENBQUM7QUFFRCxNQUFNLFVBQVUsa0JBQWtCLENBQUMsRUFBNEI7SUFDN0QsVUFBVSxHQUFHLEVBQUUsQ0FBQztBQUNsQixDQUFDO0FBRUQsTUFBTSxVQUFVLG1CQUFtQixDQUFDLEVBQTRCO0lBQzlELFdBQVcsR0FBRyxFQUFFLENBQUM7QUFDbkIsQ0FBQztBQUVELE1BQU0sVUFBVSxVQUFVLENBQUMsR0FBVztJQUNwQyxLQUFLLENBQUMsTUFBTSxDQUFDLEdBQUcsQ0FBQyxDQUFDO0FBQ3BCLENBQUM7QUFFRCxNQUFNLFVBQVUsWUFBWSxDQUFDLEdBQVc7SUFDdEMsT0FBTyxLQUFLLENBQUMsT0FBTyxDQUFDLEdBQUcsQ0FBQyxDQUFDO0FBQzVCLENBQUMifQ==